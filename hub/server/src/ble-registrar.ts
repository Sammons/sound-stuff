import * as fs from 'fs';
import * as events from 'events';

enum DeviceStatuses {
  ERROR,
  PENDING,
  SUCCESS
}

export type DeviceCmd = {
  type: 'button',
  value: boolean,
  id: number,
  name: string
}

class BLEDevice {
  constructor(private rfcomm: string, private id: number) { }
  private readStream: fs.ReadStream | null = null;
  private writeStream: fs.WriteStream | null = null;
  private status = DeviceStatuses.PENDING;
  private name: string | null = null;
  private attempts = 0;
  private listeners: Function[] = [];
  private receivedCommands = 0;
  private lastCommand: DeviceCmd | null = null;
  public getSummary() {
    return {
      rfcomm: this.rfcomm,
      id: this.id,
      name: this.name,
      status: this.status,
      receivedCommands: this.receivedCommands,
      lastCommand: this.lastCommand
    }
  }

  private clearStreams(e?: Error) {
    this.status =
      e == null
        ? DeviceStatuses.PENDING
        : DeviceStatuses.ERROR
    try {
      if (!this.readStream?.readableEnded)
        this.readStream?.destroy(e)
      if (!this.writeStream?.writableEnded)
        this.writeStream?.destroy(e);
    } catch (e) {
      // nothing, we're destroying it anyways
    }
    this.readStream = null;
    this.writeStream = null;
  }

  private parseCommand = (command: string): DeviceCmd | null => {
    if (command.startsWith('BTN')) {
      const match = command.match(/BTN\[(\d+)\]/);
      if (!match) {
        console.log('Corrupted button command from', this.rfcomm, ':', command);
        return null;
      }
      const [_, value] = match;
      return {
        type: 'button',
        value: value === '1' ? true : false,
        id: this.id,
        name: this.name!
      }
    }
    console.log('Unknown command from', this.rfcomm, ':', command);
    return null;
  }

  private configureDataListener() {
    let buf = "";
    this.readStream?.on('data', (chunk) => {
      buf += chunk.toString();
      let chunks = buf.split('\n');
      if (chunks.length > 0) {
        buf = chunks.pop()!;
      }
      if (chunks.length > 0) {
        chunks.forEach(command => {
          console.log
          if (command.startsWith('SAMMONS')) {
            // this is what the device sends periodically while
            // it is waiting to be acknowledged
            const matches = command.match(/SAMMONS_(\w+)_(\d+)/);
            if (!matches) {
              // invalid formatting
              console.log('Corrupted message received and discarded from', this.rfcomm, ':', command);
              return;
            }
            const [_, type, name] = matches
            if (type == 'BUTTON') {
              // in our made up protocol,
              // this is all the device needs to know that its been acknowledged
              this.name = `BUTTON_${name}`
              this.write(`${this.id}\n`);
            }
          } else if (command.startsWith('OK')) {
            console.log('button registered:', this.id, '=', this.name)
            this.status = DeviceStatuses.SUCCESS;
          } else {
            this.receivedCommands++;
            const parsed = this.parseCommand(command);
            this.lastCommand = parsed;
            if (parsed) {
              this.listeners.forEach(handler => {
                try {
                  handler(parsed)
                } catch (e) {
                  console.log('handler failed to process command in', this.rfcomm, ':', e)
                }
              })
            }
          }
        })
      }
    })
  }

  private openReadStream() {
    this.readStream = fs.createReadStream(this.rfcomm);
    this.readStream.on('error', (e) => {
      this.clearStreams();
    });
    this.readStream.on('close', () => {
      this.clearStreams();
    });
    this.configureDataListener();
  }

  private openWriteStream() {
    this.writeStream = fs.createWriteStream(this.rfcomm);
    this.writeStream.on('error', (e) => {
      console.log('write stream error', e, 'on device', this.rfcomm);
    })
    this.writeStream.on('close', () => {
      this.clearStreams();
    })
  }

  private write(buf: Buffer | string) {
    if (!this.writeStream) {
      this.openWriteStream();
    }
    this.writeStream?.write(buf);
  }

  /**
   * 
   * @param maxAttempts exclusive bound
   */
  public tryConnectIfNotConnected(maxAttempts: number) {
    if (this.readStream == null) {
      if (this.attempts < maxAttempts) {
        this.status = DeviceStatuses.PENDING;
        this.attempts++; // indicates how many times read stream has been established
        this.openReadStream();
      } else {
        this.status = DeviceStatuses.ERROR;
      }
    }
  }

  public getStatus() {
    return this.status;
  }
  /** How to find out if a switch is flipped or a button is pressed, etc. */
  public addListener = (handler: (cmd: DeviceCmd) => any) => {
    this.listeners.push(handler);
  }


  public removeListener = (handler: Function) => {
    this.listeners = this.listeners.filter(l => l !== handler);
  }

  public hasListener = (handler: Function) => {
    return this.listeners.includes(handler)
  }
}

export class BLERegistrar extends events.EventEmitter {
  private registrationMap = {} as {
    [key: string]: BLEDevice
  }
  /**
   * blueman-manager binds ble serial ports like /dev/rfcomm1
   * this function lists stuff in /dev and plucks out /dev/rfcomm#
   * elements.
   * */
  private getRFCommSerialPorts = () => {
    return fs.readdirSync('/dev')
      .filter(file => {
        return file.startsWith('rfcomm')
      })
      .map(file => {
        return `/dev/${file}`;
      })
  }
  private deviceCounter = 0;
  private universalDeviceEventHandler = (cmd: DeviceCmd) => {
    this.emit('cmd', cmd)
  }
  public watchForDevices = (periodMs: number = 500) => {
    setInterval(() => {
      const devices = this.getRFCommSerialPorts();
      devices.forEach(rfcomm => {
        if (!this.registrationMap[rfcomm]) {
          this.registrationMap[rfcomm] = new BLEDevice(rfcomm, this.deviceCounter);
          this.deviceCounter++;
        }
      })
      const maxConnectionAttempts = 10;
      Object.values(this.registrationMap).forEach(device => {
        device.tryConnectIfNotConnected(maxConnectionAttempts)
        if (!device.hasListener(this.universalDeviceEventHandler)) {
          device.addListener(this.universalDeviceEventHandler);
        }
      });
    }, periodMs)
    return this;
  }

  public listDevices = () => {
    return Object.values(this.registrationMap).filter(device => {
      return [DeviceStatuses.SUCCESS, DeviceStatuses.PENDING].includes(device.getStatus())
    }).map(device => {
      return device.getSummary();
    })
  }
}
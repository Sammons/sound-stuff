import * as zmq from 'zeromq';
//

export type EffectParam = {
  name: string;
  type: string;
  defaultValue?: string;
  min?: string;
  max?: string;
}

export class ZMQServer {
  private effects = {} as {
    [name: string]: {
      params: EffectParam[];
      config: {name: string; value: string}[]
    }
  }
  getEffects() {
    return Object.keys(this.effects).map(k => {
      return {
        name: k,
        params: this.effects[k].params,
        config: this.effects[k].config
      }
    });
  }
  push = zmq.createSocket('push')
  pull = zmq.createSocket('pull')
  send(msg: string) {
    this.push.send(msg);
  }
  updateConfiguration(effectName: string, config: {name: string, value: string}[]) {
    this.effects[effectName].config = config;
    const msg = config.map(param => `${param.name}=${param.value}`).join(',')
    this.send(`${effectName},${msg}`);
  }
  setup() {
    this.push.connect('tcp://127.0.0.1:4002');
    this.pull.bind('tcp://127.0.0.1:4003');

    console.log('zmq bound');
    
    this.pull.on('message', message => {
      const str = message.toString();
      console.log('message received', str)
      const pieces = str.split(',').map((s: string) => s.trim());
      let name = pieces[0];
      let params = [] as EffectParam[];
      for (let i = 1; i < pieces.length; ++i) {
        let param = pieces[i];
        let [name, type] = param.split('=');
        let defaultValue = undefined;
        let minValue = undefined;
        let maxValue = undefined;
        if (i + 1 < pieces.length && !Number.isNaN(Number(pieces[i + 1]))) {
          defaultValue = pieces[i + 1];
          ++i;
        }
        if (defaultValue != null && i + 1 < pieces.length && !Number.isNaN(Number(pieces[i + 1]))) {
          minValue = pieces[i + 1];
          ++i;
        }
        if (minValue != null && i + 1 < pieces.length && !Number.isNaN(Number(pieces[i + 1]))) {
          maxValue = pieces[i + 1];
          ++i;
        }
        params.push({
          name, type, min: minValue, max: maxValue, defaultValue
        })
      }
      // pong back the default config
      const defaultConfig = params.map(p => ({
        name: p.name,
        type: p.type,
        value: p.defaultValue || '0',
      }))
      this.effects[name] = {
        params,
        config: defaultConfig
      }
      this.updateConfiguration(name, defaultConfig);
    })
    return this;
  }
}
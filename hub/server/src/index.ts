import * as express from 'express';
import * as path from 'path';
import { BLERegistrar, DeviceCmd } from './ble-registrar';
import * as zmq from 'zeromq';
import { EffectParam, ZMQServer } from './zmq-server';
const app = express();
app.use(express.json());

const staticFileDir = path.resolve(__dirname, '../../ui/');
const port = 3000;

app.use((req, res, next) => {
  // on linux (tested on elementary os)
  // requests from the local machine appear as ::1 here
  // so filter any other requests out, we don't want web traffic
  if (req.ip !== '::1') {
    res.sendStatus(400);
    res.end();
    return next(new Error(`Invalid Requestor IP`))
  }
  next();
})

app.use(express.static(staticFileDir))

app.listen(port, () => {
  console.log(`Server listening on port ${port}. Access at http://localhost:${port}/ serving files from ${staticFileDir}`)
})

const deviceWatcher = new BLERegistrar().watchForDevices(500);

app.post('/effects', (req, res) => {
  const payload: {
    effects: {
      name: string;
      params: EffectParam[];
      config: {
        name: string;
        value: string;
      }[]
    }[]
  } = req.body;
  // TODO: UI
  payload.effects.forEach(effect => {
    zmqserver.updateConfiguration(effect.name, effect.config);
  })
})


const zmqserver = new ZMQServer().setup();

deviceWatcher.on('cmd', (cmd: DeviceCmd) => {
  zmqserver.getEffects().forEach(effect => {
    const activeValue = String(cmd.value ? 1 : 0);
    console.log('updating effect', effect.name)
    const field = effect.config.find(el => el.name == 'Active');
    if (field) {
      field.value = activeValue;
      zmqserver.updateConfiguration(effect.name, effect.config)
    }
  })
})
app.get('/summary', (req, res) => {
  res.json({
    devices: deviceWatcher.listDevices(),
    effects: zmqserver.getEffects()
  });
});
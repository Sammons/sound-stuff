import * as React from 'react';

import { observer } from 'mobx-react';
import { observable } from 'mobx';

const store = observable({
  devices: [],
  effects: []
});

let lastSummary = ''
let lastRequestCompleted = true;
let refreshAbortController = new AbortController();
setInterval(async () => {
  if (!lastRequestCompleted) {
    refreshAbortController.abort();
    lastRequestCompleted = true;
    refreshAbortController = new AbortController();
    return;
  }
  lastRequestCompleted = false;
  const devices = await fetch('/summary', {
    method: 'GET',
    signal: refreshAbortController.signal
  }).then(async res => {
    const next = await res.text();
    if (next === lastSummary) {
      return;
    }
    lastSummary = next
    const summary = JSON.parse(next);
    store.devices = summary.devices;
    store.effects = summary.effects;
  }).finally(() => {
    lastRequestCompleted = true;
  })
}, 100);

let updateAbortController: AbortController = new AbortController();
let pendingUpdate = false;
const updateServer = async() => {
  if (pendingUpdate) {
    updateAbortController.abort();
    updateAbortController = new AbortController();
    pendingUpdate = false;
  }
  pendingUpdate = true;
  fetch('/effects', {
    body: JSON.stringify({
      effects: store.effects.map(e => e)
    }),
    headers: {
      'content-type': 'application/json'
    },
    method: 'POST',
    signal: updateAbortController.signal
  }).finally(() => {
    pendingUpdate = false;
  })
}

export const Landing = observer(() => <div>
  <div>Welcome!</div>
  <div>
    Devices:
    <ul>
      {store.devices.map((d: {
        rfcomm: string,
        name: string;
        status: string;
        lastCommand: {}
      }) => {
        return <li key={d.name}>
          <div>
            <div>name: {d.name}</div>
            <div>Serial Port File: {d.rfcomm}</div>
            <div>Position: {JSON.stringify(d.lastCommand)}</div>
            <div>Status: {String(d.status) === String(2) ? 'Connected' : 'Not Connected'}</div>
          </div>
        </li>
      })}
    </ul>
    Effects:
    <ul>
      {store.effects.map((d: {
        name: string;
        params: { name: string, type: string, min: string, max: string, defaultValue: string }[]
        config: { name: string, value: string }[]
      }) => {
        return <li key={d.name}>
          <div>
            <div>name: {d.name}</div>
            {/* <div>Params: {JSON.stringify(d.params)}</div> */}
            {
              d.params.map(param => {
                const config = d.config.find(c => c.name === param.name)!;
                if (param.type == 'bool') {
                  return <div key={param.name}>
                    <label>{param.name}</label>
                    <input type="checkbox" checked={config.value === '1'} />
                  </div>
                }
                if (param.type == 'number') {
                  return <div key={param.name}>
                    <div><label>{param.name} ({config.value})</label></div>
                    <div>
                    <input type="range" min={param.min} max={param.max} value={config.value}
                      onChange={((e) => {
                        config.value = e.target.value
                        updateServer();
                      })} />
                    </div>
                  </div>
                }
                return ''
              })
            }
          </div>
        </li>
      })}
    </ul>
  </div>
</div>)
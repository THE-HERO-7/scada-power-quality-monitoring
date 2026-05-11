const { contextBridge, ipcRenderer } = require('electron');

contextBridge.exposeInMainWorld('api', {
    onSerial: (cb) => ipcRenderer.on('serial-data', (s, d) => cb(d)),
    onBackend: (cb) => ipcRenderer.on('backend-data', (s, d) => cb(d))
});

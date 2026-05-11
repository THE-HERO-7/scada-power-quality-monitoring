const { app, BrowserWindow } = require('electron');
const path = require('path');
const { SerialPort } = require('serialport');
const { ReadlineParser } = require('@serialport/parser-readline');
const { spawn } = require('child_process');

let mainWindow;
let pyProcess;

app.whenReady().then(() => {
  mainWindow = new BrowserWindow({
    width: 1200,
    height: 800,
    webPreferences: {
      preload: path.join(__dirname, 'preload.js'),
      contextIsolation: true,
      nodeIntegration: false
    }
  });

  mainWindow.loadFile('index.html');

  // ---------- SERIAL ----------
  const port = new SerialPort({ path: 'COM8', baudRate: 115200 });
  const parser = port.pipe(new ReadlineParser({ delimiter: '\n' }));

  port.on('error', (err) => console.error('Serial Error:', err.message));

  // ---------- PYTHON (Path Fixed for your structure) ----------
  // '../' looks outside scada_electron into the 'src' folder
  const pyPath = path.join(__dirname, '../scada_backend.py');
  pyProcess = spawn('python', [pyPath]);

  pyProcess.on('error', (err) => console.error('Python Launch Error:', err.message));

  // ---------- DATA FLOW ----------
  parser.on('data', (line) => {
    const clean = line.trim();
    if (!clean) return;
    
    mainWindow.webContents.send('serial-data', clean);
    pyProcess.stdin.write(clean + '\n');
  });

  pyProcess.stdout.on('data', (data) => {
    const output = data.toString().trim();
    mainWindow.webContents.send('backend-data', output);
});

  pyProcess.stderr.on('data', (data) => console.error(`PyErr: ${data}`));
});

app.on('window-all-closed', () => {
  if (pyProcess) pyProcess.kill();
  app.quit();
});

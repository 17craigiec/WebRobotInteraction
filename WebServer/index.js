var http = require('http');
var fs = require('fs');

const hostname = '127.0.0.1';
const port = 3000;

const {spawn} = require('child_process');

var server = http.createServer(function (request, response) {
  if (request.url.includes("D_")) {
    command = request.url.split("_");

    const childPythonDrive = spawn('python3', ['PythonScripts/DriveInterface.py', parseInt(command[1]), parseInt(command[2])]);

    // childPythonDrive.stdout.on('data', (data) => {
    //   console.log(data.toString());
    // });

    // childPythonDrive.stderr.on('data', (data) => {
    //   console.log(data.toString());
    // });

    // childPythonDrive.on('close', (code) => {
    //   console.log(code.toString());
    // });

    // Close up shop
    response.end();
  }
  else if (request.url.includes("A_")) {
    command = request.url.split("_");

    const childPythonArm = spawn('python3', ['PythonScripts/ArmInterface.py', parseInt(command[1]), parseInt(command[2])]);

    response.end();
  }
  else {
    fs.readFile('./' + request.url, function(err, data) {
      if (!err) {
        var dotoffset = request.url.lastIndexOf('.');
        var mimetype = dotoffset == -1
                        ? 'text/plain'
                        : {
                            '.html' : 'text/html',
                            '.ico' : 'image/x-icon',
                            '.jpg' : 'image/jpeg',
                            '.png' : 'image/png',
                            '.gif' : 'image/gif',
                            '.css' : 'text/css',
                            '.js' : 'text/javascript'
                            }[ request.url.substr(dotoffset) ];
        response.setHeader('Content-type' , mimetype);
        response.end(data);
        console.log( request.url, mimetype );
      }
      else {
        console.log ('file not found: ' + request.url);
        response.writeHead(404, "Not Found");
        response.end();
      }
    });
  }
})

server.listen(port, hostname, () => {
  console.log(`Server running at http://${hostname}:${port}/WebRobot.html`);
});

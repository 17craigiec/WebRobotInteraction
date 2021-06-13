var http = require('http');
var fs = require('fs');

//const hostname = '127.0.0.1';
var args = process.argv.slice(2);
var hostname = '0.0.0.0';
console.log(args.length);
if (args.length < 1){
  hostname = '10.0.0.14';
}else{
  hostname = '10.42.0.69';
}
const port = 3000;

const {spawn} = require('child_process');

var server = http.createServer(function (request, response) {
  if (request.url[1] == 'D' && request.url.includes("D_")) {
    console.log(request.url)
    command = request.url.split("_");

    const childPythonDrive = spawn('python3', ['PythonScripts/DriveInterface.py', parseInt(command[1]), parseInt(command[2])]);

    childPythonDrive.stdout.on('data', (data) => {
      console.log(data.toString());
    });

    childPythonDrive.stderr.on('data', (data) => {
      console.log(data.toString());
    });

    // childPythonDrive.on('close', (code) => {
    //   console.log(code.toString());
    // });

    // Close up shop
    response.end();
  }
  else if (request.url.includes("A_")) {
    console.log(request.url);
    command = request.url.split("_");
    console.log(command[1]);
    console.log(command[2]);
    console.log("======================= PYTHON =======================");

    if(Number.isInteger(command[1]) || Number.isInteger(command[2])){
      command[1] = parseInt(command[1]);
      command[2] = parseInt(command[2]);
      command[3] = parseInt(command[3]);
      command[4] = parseInt(command[4]);
    }
    const childPythonArm = spawn('python3', ['PythonScripts/ArmInterface.py', command[1], command[2], command[3], command[4]]);

    childPythonArm.stdout.on('data', (data) => {
      console.log(data.toString());
    });

    childPythonArm.stderr.on('data', (data) => {
      console.log(data.toString());
    });

    childPythonArm.on('close', (code) => {
      console.log(code.toString());
    });

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

var http = require('http'); // Import Node.js core module

var OPEN_LOCK = "open_lock";
var CLOSE_LOCK = "close_lock";
var NONE_LOCK = "none_lock";

var doors = {};
doors["back-door"] = {"door_open":true, "door_locked":true};

var action = "none";

var server = http.createServer(function (req, res) {   //create web server
  var data = [];
    req.on('data', chunk => {
      data.push(chunk)
    });

    if (req.url == '/') { //check the URL ure the current request

        // set response header
        res.writeHead(200, { 'Content-Type': 'text/html' });

        // set response content
        res.write('<html><body><p>This is home Page.</p></body></html>');
        res.end();

    }
    else if (req.url == "/door-status") {
        var door_name = JSON.parse(data.toString()).name;
        res.writeHead(200, { 'Content-Type': 'text/json' });
        res.write(JSON.stringify(doors[door_name]));
        res.end();

    }
    else if (req.url == "/set_door_status") {

      req.on('end', () => {
//        console.log(data.toString());
        var status = JSON.parse(data.toString());
        door_open = status.door_open;
        door_locked = status.door_locked;
        console.log("status was");
        console.log(status);
        if(action == OPEN_LOCK && !door_locked){
          action = NONE_LOCK;
        }else if(action == CLOSE_LOCK && door_locked){
          action = NONE_LOCK;
        }

        res.writeHead(200, { 'Content-Type': 'text/plain' });
        res.write(JSON.stringify({door_open,door_locked,action}));
        res.end();
      });



    }
    else if (req.url = "/open_lock"){
      //
      action = OPEN_LOCK;
    }else if (req.url = "/close_lock"){

    }
    else
        res.end('Invalid Request!');

});

server.listen(5000); //6 - listen for any incoming requests

console.log('Node.js web server at port 5000 is running..')

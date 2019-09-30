var http = require('http'); // Import Node.js core module
var url = require('url');
var fs = require('fs');

var OPEN_LOCK = "open_lock";
var CLOSE_LOCK = "close_lock";
var NONE_LOCK = "none_lock";
var TIMEOUT_LOCK = 20000; //in ms

var OPEN_HOSE = "open_hose";
var CLOSE_HOSE = "close_hose";

var doors = {};
var hoses = {};

function add_door(door_name){
  if (typeof doors[door_name] === 'undefined') {
    doors[door_name] = {};
    doors[door_name].action = NONE_LOCK;
    doors[door_name].sent_message = false;
  }
}

function add_hose(hose_name){
  if (typeof hoses[hose_name] === 'undefined') {
    hoses[hose_name] = {};
    hoses[hose_name].hoses = [CLOSE_HOSE,CLOSE_HOSE,CLOSE_HOSE,CLOSE_HOSE];
    //hoses[hose_name].hoses = [OPEN_HOSE,OPEN_HOSE,OPEN_HOSE,OPEN_HOSE];
    hoses[hose_name].sent_message = false;
  }
}



var server = http.createServer(function (req, res) {   //create web server
  var data = [];
    req.on('data', chunk => {
      data.push(chunk)
    });

    if (req.url == '/test') { //check the URL ure the current request

        // set response header
        res.writeHead(200, { 'Content-Type': 'text/html' });

        // set response content
        res.write('<html><body><p>This is home Page.</p></body></html>');
        res.end();

    }
    else if (req.url == "/door-status") {
        res.writeHead(200, { 'Content-Type': 'text/json' });
        res.write(JSON.stringify(doors));
        res.end();

    }
    else if (req.url == "/set_door_status") {

      req.on('end', () => {
//        console.log(data.toString());
        var status = JSON.parse(data.toString());
        door_name = status.door_name;
        add_door(door_name);

        doors[door_name].door_open = status.door_open;
        doors[door_name].door_locked = status.door_locked;
        doors[door_name].door_unlocked = status.door_unlocked;

        console.log("status was");
        console.log(status);
        if(doors[door_name].sent_message && doors[door_name].action == OPEN_LOCK && !doors[door_name].door_locked){
          doors[door_name].action = NONE_LOCK;

        }else if(doors[door_name].sent_message && doors[door_name].action == CLOSE_LOCK && doors[door_name].door_locked){
          doors[door_name].action = NONE_LOCK;

        }

        res.writeHead(200, { 'Content-Type': 'text/plain' });
        res.write(doors[door_name].sent_message ? NONE_LOCK : doors[door_name].action);
        if(doors[door_name].action != NONE_LOCK){
          doors[door_name].sent_message = true;
        }
        res.end();
      });



    }else if (req.url == "/set_hose_status") {

      req.on('end', () => {
//        console.log(data.toString());
        var status = JSON.parse(data.toString());
        hose_name = status.hose_name;
        add_hose(hose_name);


        console.log("status was");
        console.log(status);

        res.writeHead(200, { 'Content-Type': 'text/plain' });

        res.write(hoses[hose_name].hoses[0] + "," +
                  hoses[hose_name].hoses[1] + "," +
                  hoses[hose_name].hoses[2] + "," +
                  hoses[hose_name].hoses[3]);

        if(hoses[hose_name].action != NONE_LOCK){
          hoses[hose_name].sent_message = true;
        }
        res.end();
      });



    }else if (req.url.startsWith( "/lock_status")){
      var q = url.parse(req.url, true).query;

      res.writeHead(200, { 'Content-Type': 'text/json' });
      res.write(JSON.stringify({"status": ((doors[q.door_name].action == NONE_LOCK) ? true : false)}));
      res.end();

    }else if (req.url.startsWith( "/open_lock")){
      //
      req.on('end', () => {
        var q = url.parse(req.url, true).query;
        add_door(q.door_name);
        console.log("door name was " + q.door_name);
        doors[q.door_name].action = OPEN_LOCK;
        doors[q.door_name].sent_message = false;

        res.writeHead(200, { 'Content-Type': 'text/json' });
        res.write(JSON.stringify({"status": true}));
        res.end();

        });


    }else if (req.url.startsWith( "/close_lock")){
      req.on('end', () => {
        var q = url.parse(req.url, true).query;
        add_door(q.door_name);
        doors[q.door_name].action = CLOSE_LOCK;
        doors[q.door_name].sent_message = false;

        res.writeHead(200, { 'Content-Type': 'text/json' });
        res.write(JSON.stringify({"status": true}));
        res.end();

      });

    }else if (req.url.startsWith( "/open_hose")){
      //
      req.on('end', () => {
        var q = url.parse(req.url, true).query;
        add_hose(q.hose_name);
        console.log("hose name was " + q.hose_name);

        hoses[q.hose_name].hoses[q.hose_number] = OPEN_HOSE;
    //    if(q.time_open){
    //      setTimeout(function(){

    // this code will only run when time has ellapsed
      //      }, n * 1000);
      //  }
        res.writeHead(200, { 'Content-Type': 'text/json' });
        res.write(JSON.stringify({"status": true}));
        res.end();

        });


    }else if (req.url.startsWith( "/close_hose")){
      req.on('end', () => {
        var q = url.parse(req.url, true).query;
        add_hose(q.hose_name);
        hoses[q.hose_name].hoses[q.hose_number] = CLOSE_HOSE;
        res.writeHead(200, { 'Content-Type': 'text/json' });
        res.write(JSON.stringify({"status": true}));
        res.end();

      });

    }
    else{

      var stream = fs.createReadStream("index.html");
      stream.on('error', function() {
        res.writeHead(404);
        res.write(req.url);
        res.end();
      });
      stream.pipe(res);
    }

});

server.listen(5000); //6 - listen for any incoming requests

console.log('Node.js web server at port 5000 is running..')

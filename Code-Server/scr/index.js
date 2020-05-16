//require dependence
const express = require('express');
const app = express();
const mongoose = require('mongoose');
const bodyParser = require('body-parser');
const http = require('http').Server(app);
const io = require('socket.io')(http);
//const mqtt = require('mqtt');

///const mosca = require('mosca');
//const ascoltatori = require('ascoltatori');//
//"start": "node scr/index.js",
/*--------------------------------------------------------------------------------------------------------- */
//let mqtt = require('../mosca/mosca');
let cloudmqtt = require('../mosca/cloudmqtt');
cloudmqtt.connect();
/*--------------------------------------------------------------------------------------------------------- */

//Body-Parser extracts the entire body portion 
//of an incoming request stream and exposes it on req.body
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({extended: false}))
app.use(express.static('public'));
app.use(express.json({limit:"100Kb"})); //gioi han kich thuoc file gui len

/*--------------------------------------------------------------------------------------------------------- */

//model connection
require('../scr/router/schema');
let model_data_fromweb = new mongoose.model('Input_from_iot_devices');
require('../scr/router/schema_1day')
let save_data_one_day = mongoose.model('data_one_day');
require('../scr/router/schema_list_device')
let iot_divice_infos  = mongoose.model('iot_divice_infos ');
/*--------------------------------------------------------------------------------------------------------- */

//Connect data-market IOTA
const fetch = require('node-fetch');
//const { publish } = require('./iota/iota');
//const { debug, serverUrl } = require('./iota/config.json');

/*--------------------------------------------------------------------------------------------------------- */

//create a connection with iosocket
io.on('connection', ()=> {
    console.log('a user is connected - io');
})
/*--------------------------------------------------------------------------------------------------------- */


//connect port
const portNumber = process.env.PORT || 3000;
http.listen(portNumber, () => {
    console.log('server is running on port', portNumber);
  });

/*--------------------------------------------------------------------------------------------------------- */

//request list divice
app.get('/divice-connect', (req, res) => {
  iot_divice_infos.find({},(err, messages)=> {
        if(err)
          console.log(err);
        else
          res.send(messages);
      })
    })

//request data from Id divice
app.get('/divice-connect/:idandoutput', (req, res) => {
      const user_time = req.params.idandoutput.split(',');
      //let user = req.params.idandoutput;
      const user = user_time[0];
      const time_grap = user_time[1];
      console.log(user);
      const get_day = new Date();
      let get_timestamp = get_day.getTime(); 
      if(time_grap == 31){
        get_timestamp = Math.round((get_timestamp - 2678400000)/1000);
        save_data_one_day.find( {id: user, time:{ $gte : get_timestamp} }, (err, dataout)=>{
          if(err)
            console.log('error 31 day', err);
          else
            res.send(dataout);
        })
      }
      else{
        get_timestamp = Math.round((get_timestamp - 172800000 )/1000);
        model_data_fromweb.find( {id: user, time:{ $gte : get_timestamp} }, (err, dataout)=>{
          if(err)
            console.log('error 2 day', err);
          else
            res.send(dataout);
        })
      }

      
    })

//request link to Iota tangle
app.get('/divice-iota/:iddivice', (req, res)=>{
  let id_divice_request = req.params.iddivice ;
  iot_divice_infos.find({id_device: id_divice_request},(err, messages)=> {
    if(err)
      console.log(err);
    else
      res.send(messages[0].mac);
  })
})

//receive data envi input form web, for test
app.post('/messages', async (req, res) => {
      try{
        var message = new model_data_fromweb(req.body);
        message.time =  Math.round(+new Date()/1000);
        io.emit('message', req.body);
        var savedMessage = await message.save()
          console.log('Saved data from web');
          res.sendStatus(200);
      }
      catch (error){
        res.sendStatus(500);
        return console.log('error',error);
      }
      // finally{
      //   console.log('Message Posted')
      // }
    
    });

//dung mqtt
//   client.subscribe("/server-request");

//receive device info from web input form
app.post('/divice-connect-info', async (req, res) => {
      try{
       // var message = new Info_of_divice(req.body);
       var message = new iot_divice_infos(req.body);
        io.emit('message', req.body);
        console.log("Request data from id =",message);
        var savedMessage = await message.save()
          console.log('saved info of IOT divice from web');
          res.sendStatus(200);
      }
      catch (error){
        res.sendStatus(500);
        return console.log('error',error);
      }
      // finally{
      //   console.log('Connected to IOT divice')
      // }
    
    })

//loc cac gia tri khong hop le
    /**
  var invalidEntries = 0;

function isNumber(obj) {
  return obj !== undefined && typeof(obj) === 'number' && !isNaN(obj);
}

function filterByID(item) {
  if (isNumber(item.id) && item.id !== 0) {
    return true;
  } 
  invalidEntries++;
  return false; 
}

var arrByID = arr.filter(filterByID);

console.log('Filtered Array\n', arrByID); 
     */

  
/*--------------------------------------------------------------------------------------------------------- */


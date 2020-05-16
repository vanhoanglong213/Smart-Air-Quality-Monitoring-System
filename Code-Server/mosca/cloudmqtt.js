const mqtt = require('mqtt');
const mongoose = require('mongoose');
const { publish } = require('../scr/iota/iota');


let data_y_axis_co2 = [];
let data_y_axis_o3 = [];
let data_y_axis_tvoc = [];
let data_y_axis_pm25 = [];
let data_y_axis_pm10 = [];
let data_y_axis_aqi = [];
let data_y_axis_tem = [];
let data_y_axis_humi = [];
//cloud mqtt 
/*conect database-----------------------------------------------------------------------*/ 
require('../scr/router/schema');
let model_data_fromdivice = mongoose.model('Input_from_iot_devices');
require('../scr/router/schema_1day')
let save_data_one_day = mongoose.model('data_one_day');
require('../scr/router/schema_list_device')
let iot_divice_infos  = mongoose.model('iot_divice_infos ');
// const Data_from_divice=  mongoose.model("Input_from_iot_devices",
// { 
//     id_device: String,
//     pm10: String,
//     pm25: String,
//     co2: String,
//     tvoc: String,
//     temp: String,
//     humd: String,
//     o3: String,
//     time: String
// });

/*--------------------------------------------------------------------------------------*/ 
// let mqtt_url =  'mqtt://tailor.cloudmqtt.com';
// var options = {
//         port: 15587,
//         //host: 'mqtt://tailor.cloudmqtt.com',
//         clientId: 'CLKK_mqtt',
//         username: 'gaxlylcj',
//         password: 'YYhH75v1oDzx',
//         keepalive: 1000,
//         retain: false
// };
// let client = mqtt.connect(mqtt_url,options );

let mqtt_mos_url = 'mqtt://test.mosquitto.org'
var options = { port: 1883};
let client = mqtt.connect(mqtt_mos_url,options );

let count = 0;
client.on('connect', () => { 
        

});
/*--------------------------------------------------------------------------------------*/ 
const url = "mongodb://localhost:27017/database_local";
const MongoClient = require('mongodb').MongoClient;

const connect_mogodb = mongoose.connect(url, { useUnifiedTopology: true, useNewUrlParser: true }, (err) =>{
    if(err)
        console.log(err);
    else
        console.log("Conected to local data");
});
/*--------------------------------------------------------------------------------------*/ 

/*CONNECT TO CLOUD MONGO ATLAT DATABASE */

// const ulr_mongodb = `mongodb+srv://Airquality-input_1:fY6UTAYNUJto5kmH@cachep-iot-3agua.mongodb.net/test?retryWrites=true&w=majority`;
// mongoose.connect(ulr_mongodb, { useUnifiedTopology: true , useNewUrlParser: true }, (err) =>{
//     if(err)
//         console.log('mongodb is not connected ', err);
//     else
//         console.log('Conected sucsess to DB Cloud atlas')
// });
/*--------------------------------------------------------------------------------------*/ 
let connect = function runCloudMqtt() {

/*--------------------------------------------------------------------------------------*/ 

client.subscribe('/divice-connect-info', () => {      
    console.log('subscribe to "/divice-connect-info"' );
});
client.subscribe('/divice-connect-data/#', () => {
    console.log('subscribe to "/divice-connect-data/#" ' );
});

/*--------------------------------------------------------------------------------------*/ 
let flag_send_publish = [], flag_interval_1hour = 0;
let id_list_iota = [], key_list_iota = [];
let id_iota, key_iota;
/*--------------------------------------------------------------------------------------*/ 
client.on('message', async function(topic, message) {
        let i, save_data=0;
        let length_array_id = flag_send_publish.length;
        if(length_array_id != 0)
        {
            for(i = 0; i < flag_send_publish.length; i++){
                let same_topic = '/divice-connect-data/'+ flag_send_publish[i];
                if(topic === same_topic){
                   // console.log("same, save data to database");
                    save_data = 1;
                    id_iota = id_list_iota[i];
                    key_iota = key_list_iota[i];
                    flag_send_publish.splice(i,1);
                    id_list_iota.splice(i,1);
                    key_list_iota.splice(i,1);
                }
            }
        }
        else
        {
            flag_interval_1hour = 0;
            save_data = 0;
        }
        if(save_data){
                //save database
                let mes_save = JSON.parse(message.toString());
                let { pm10, pm25, co2, tvoc,temp, humd, o3 } = mes_save;
                const payload = {pm10, pm25, co2, tvoc,temp, humd, o3} ;
                await publish(payload, id_iota, key_iota);
                //console.log(mes_save);
                    try{
                        let Message = new model_data_fromdivice(mes_save);
                        Message.time =  Math.round(+new Date()/1000);
                        var savedMessage = await Message.save()
                        console.log( new Date().toLocaleString())
                        console.log('Saved data from IOT divice');
                    }
                    catch (error){
                        return console.log('error',error);
                    }
              save_data = 0;
        }
    });
/*--------------------------------------------------------------------------------------*/ 
//every 5min, server will request data from sensor ultil all send data to server
//if not, it request every 5min.
async function publictoiota(payload,id_iota,key_iota) {
    await publish(payload,id_iota,key_iota);
};

function check_interval_publish(){ 
    
    setInterval( function(){
        let i;
        if(flag_interval_1hour){
            if(flag_send_publish.length >=1)
            {
                console.log("Send Request to all device every 5 min");
                for(i =0; i < flag_send_publish.length; i ++ )
                    publish_mqtt(flag_send_publish[i]);
            }
            else
                flag_interval_1hour = 0;
         }
    }, 20000);
   

}
//check_interval_publish();

//Every 1 hour, server will request data from all sensor have id in database.
setInterval( function(){
        request_data_from_divice();
        console.log( new Date().toLocaleString())
        console.log("Send request to all divice every 5min");
        flag_interval_1hour = 1;
        const get_day = new Date();
        let get_hours = get_day.getHours(); 
        let get_timestamp = get_day.getTime(); 
        if(get_hours === 0) // = gio thuc te  0 - 23 
        {
            //luu data 1 ngay
            save_data_in_one_day(get_timestamp);
            console.log("save data 1 day finish");
        }
        }, 300000); //milisecond 3600000

/*--------------------------------------------------------------------------------------*/ 

function publish_mqtt(id_to_publish){
    let url_subscribe = '/server-request/'+ id_to_publish;
    client.publish(url_subscribe, '89');
}

function request_data_from_divice(){
       let i;
       flag_send_publish = [];
       id_list_iota = [];
       key_list_iota = [];
       iot_divice_infos.find( {})
            .then( (data_out) => {
            for(i = 0; i < data_out.length; i ++){
                publish_mqtt(data_out[i].id_device);
                flag_send_publish.push(data_out[i].id_device);
                id_list_iota.push(data_out[i].mac);
                key_list_iota.push(data_out[i].secretKey);
            }
            // console.log(flag_send_publish);
            // console.log(id_list_iota);
            // console.log(key_list_iota);
        })  
        .catch( (err) => {
            console.log('erro public: ', err);
      //  client.publish('/server-request', '89');
        });
    }
/*--------------------------------------------------------------------------------------*/ 

function save_data_in_one_day(timestamp){
        let j;
        let time_to_querry = Math.round((timestamp - 86400000) /1000); //querry all data in last day
        console.log('time querry = ', time_to_querry);
                iot_divice_infos.find().then( (dataout) =>{
                    for(j =0; j < dataout.length; j++)
                    {
                        let id_querry_find = dataout[j].id_device;
                        model_data_fromdivice.find({id: id_querry_find, time:{ $gte : time_to_querry}})
                        .then( (json_data_out_a)=>{
                            push_data_array(json_data_out_a, id_querry_find, time_to_querry);
                           // k =  (json_data_out_a.length -1);
                         })
                        .catch((err) => {
                            console.log('erro 0: ', err)});
                    }
                });
    }
function push_data_array(json_data_out, id, time_save){
                    data_y_axis_co2 = [];
                    data_y_axis_o3 = [];
                    data_y_axis_tvoc= [];
                    data_y_axis_pm25= [];
                    data_y_axis_pm10= [];
                    data_y_axis_aqi = [];
                    data_y_axis_tem = [];
                    data_y_axis_humi = [];
                    let i;
                    for(i = 0; i < json_data_out.length; i ++){
                            
                            data_y_axis_co2.push(parseInt(json_data_out[i].co2));
                            data_y_axis_o3.push( parseInt(json_data_out[i].o3));
                            data_y_axis_tvoc.push(parseInt(json_data_out[i].tvoc));
                            data_y_axis_pm25.push(parseInt(json_data_out[i].pm25));
                            data_y_axis_pm10.push(parseInt(json_data_out[i].pm10));
                            data_y_axis_aqi.push(parseInt(json_data_out[i].aqi));
                            data_y_axis_tem.push(parseInt(json_data_out[i].tem));
                            data_y_axis_humi.push(parseInt(json_data_out[i].humi));
                            
                    }
                    let reducer = function (accumulator, currentValue) {
                        return accumulator + currentValue;
                      };

                    const sum1 = data_y_axis_pm10.reduce(reducer, 0);
                    let average1 = Math.round(sum1 / data_y_axis_pm10.length);
                   
                    const sum2 = data_y_axis_pm25.reduce(reducer, 0);
                    let average2 = Math.round(sum2 / data_y_axis_pm25.length);

                    const sum3 =data_y_axis_co2.reduce(reducer, 0);
                    let average3 = Math.round(sum3 / data_y_axis_co2.length);
                    
                    const sum4 = data_y_axis_tvoc.reduce(reducer, 0);
                    let average4 = Math.round(sum4 / data_y_axis_tvoc.length);

                    const sum5 = data_y_axis_o3.reduce(reducer, 0);
                    let average5 = Math.round(sum5 / data_y_axis_o3.length); 

                    const sum6 = data_y_axis_tem.reduce(reducer, 0);
                    let average6 = Math.round(sum6 / data_y_axis_tem.length);

                    const sum7 = data_y_axis_humi.reduce(reducer, 0);
                    let average7 = Math.round(sum7 / data_y_axis_humi.length); 
                    
                    let aqi_max = Math.max(...data_y_axis_aqi);

                    // let json_save = {
                    //     aqi: aqi_max.toString(),
                    //     pm10: average1.toString(),
                    //     pm25: average2.toString(),
                    //     co2: average3.toString(),	
                    //     tvoc: average4.toString(),
                    //     temp: average6.toString(),
                    //     humd: average7.toString(),
                    //     o3:average5.toString(),
                    //     time: time_save.toString(),
                    //     id: id.toString()
                    // }
                    let json_save = {
                            aqi: aqi_max,
                            pm10: average1,
                            pm25: average2,
                            co2: average3,	
                            tvoc: average4,
                            temp: average6,
                            humd: average7,
                            o3:average5,
                            time: time_save,
                            id: id
                        }

                    json_save = JSON.stringify(json_save);
                    json_save = JSON.parse(json_save);
                    console.log("data 1 day:" );
                    console.log(json_save);
                   //save to database 1 day
                   save_1day(json_save);
    }

async function save_1day(datain){
        let Message_1day = new save_data_one_day(datain);
        var savedMessage =  await Message_1day.save()
        console.log('Saved data to database 1 day');
    }
};


exports.connect = connect;

   
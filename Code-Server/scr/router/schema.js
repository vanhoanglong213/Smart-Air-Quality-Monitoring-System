const mongoose = require('mongoose');

let data_from_divece_schema = mongoose.Schema({
    aqi: String,
    pm10: String,
    pm25: String,
	co2: String,	
    tvoc: String,
    temp: String,
    humd: String,
    o3: String,
    time: String,
    id: String,
    secretKey: String
});



module.export =  mongoose.model("Input_from_iot_devices",data_from_divece_schema);


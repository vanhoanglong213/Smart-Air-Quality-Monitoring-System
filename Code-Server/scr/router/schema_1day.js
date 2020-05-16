const mongoose = require('mongoose');

let save_one_day_data = mongoose.Schema({
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
    secret: String
});



module.export =  mongoose.model("data_one_day",save_one_day_data);

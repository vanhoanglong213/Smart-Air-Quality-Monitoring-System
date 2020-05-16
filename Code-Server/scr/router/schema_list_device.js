const mongoose = require('mongoose');

const Info_of_divice =  mongoose.Schema(
{ 
  id_device: String, //khong duoc trung
  mac: String,//khong duoc trung
  secretKey: String,
  latitude: String,
  longitude: String
});



module.export =  mongoose.model("iot_divice_infos ",Info_of_divice);

    let array = [];
    const xlabel = ['co2','o3','tvoc','pm10','pm25'];
    let data_y_axis = [];
    var socket = io();

    $(() => {
        // $("#send").click(()=>{
        //     sendMessage({id_device: $("#id").val(),co2: $("#co2").val(),
        //     o3: $("#o3").val(), tvoc: $("#tvoc").val(),
        //     pm25: $("#pm25").val(), pm10: $("#pm10").val() });
            
        // })  
        $("#send-info").click( ()=>{
            sendInfo({id_device: $("#id_divice_1").val(),
                      mac: $("#sensorname").val(),
                      secretKey: $("#secretKey").val(), 
                      latitude: $("#latitude").val(),
                      longitude: $("#lontitude").val()}); 
        
        });

    });

// function sendMessage(message){
//       $.post('/messages', message, (data, err) =>{
//         if(data !== 'OK'){
//             console.log('Error send data', err);
//             console.log(data);
//         }
//         else{
//             alert ("Send infor device successfully");
//             ClearFields1();           
//         }

//         });
  
//     }
    // function ClearFields1(){
    //     document.getElementById("id_device").value = "";
    //     document.getElementById("co2").value = "";
    //     document.getElementById("o3").value = "";
    //     document.getElementById("tvoc").value = "";
    //     document.getElementById("pm10").value = "";
    //     document.getElementById("pm25").value = "";
    // };
    
function sendInfo(message){
      $.post('/divice-connect-info', message, (data,err) =>{
    if(data !== 'OK'){
            console.log('Error send data', err);
     }
      else{
            alert ("Send infor device successfully");
            ClearFields();
            console.log(data);
            }
      })
    
    }
    function ClearFields(){
        document.getElementById("id_divice_1").value = "";
        document.getElementById("sensorname").value = "";
        document.getElementById("secretKey").value = "";
        document.getElementById("latitude").value = "";
        document.getElementById("lontitude").value = "";
    };


// Called after form input is processed
function startConnect() {
    // Generate a random client ID
    clientID = "CLKK-Web-" + parseInt(Math.random() * 100);

    // Fetch the hostname/IP address and port number from the form
    host = document.getElementById("host").value;
    port = document.getElementById("port").value;

    // Print output for the user in the messages div
    document.getElementById("messages").innerHTML += '<span>Connecting to: ' + host + ' on port: ' + port + '</span><br/>';
    document.getElementById("messages").innerHTML += '<span>Using the following client value: ' + clientID + '</span><br/>';

    // Initialize new Paho client connection
    clkkmqtt = new Paho.MQTT.Client(host, Number(port), clientID);

    // Set callback handlers
    clkkmqtt.onConnectionLost = onConnectionLost;
    clkkmqtt.onMessageArrived = onMessageArrived;

    // Connect the client, if successful, call onConnect function
    clkkmqtt.connect({ 
        onSuccess: onConnect,
    });
}

// Called when the client connects
function onConnect() {
    // Fetch the MQTT topic from the form
    topic = document.getElementById("topic").value;

    // Print output for the user in the messages div
    document.getElementById("messages").innerHTML += '<span>Subscribing to: ' + topic + '</span><br/>';

    // Subscribe to the requested topic
    clkkmqtt.subscribe(topic);
}

// Called when the client loses its connection
function onConnectionLost(responseObject) {
    document.getElementById("messages").innerHTML += '<span>ERROR: Connection lost</span><br/>';
    if (responseObject.errorCode !== 0) {
        document.getElementById("messages").innerHTML += '<span>ERROR: ' + + responseObject.errorMessage + '</span><br/>';
    }
    console.log(responseObject);
}

function convertTimestamphour(timestamp) {
    var d = new Date(timestamp * 1000), // Convert the passed timestamp to milliseconds
        hh = d.getHours(),
        h = hh,
        min = ('0' + d.getMinutes()).slice(-2),     // Add leading 0.
        ampm = 'AM',
        time,
        yyyy = d.getFullYear(),
        mm = ('0' + (d.getMonth() + 1)).slice(-2),  // Months are zero based. Add leading 0.
        dd = ('0' + d.getDate()).slice(-2);         // Add leading 0.

    if (hh > 12) {
        h = hh - 12;
        ampm = 'PM';
    } else if (hh === 12) {
        h = 12;
        ampm = 'PM';
    } else if (hh == 0) {
        h = 12;
    }
    
    // ie: 2014-03-24, 3:00 PM
    time =  dd + '/' + mm + '/' + yyyy +','+ h + ':' + min + ' ' + ampm;
    return time;

}

// Called when a message arrives
function onMessageArrived(message) {
    
    let mes_save = JSON.parse(message.payloadString);
    console.log("onMessageArrived: " + mes_save.id);
    let nowtimestamp =  Math.round(+new Date()/1000);
    let day = convertTimestamphour(nowtimestamp);
    document.getElementById("messages").innerHTML += '<span>ID : ' + mes_save.id + '  | ' + day + '</span><br/>';
   // document.getElementById("messages").innerHTML += '<span>Topic: ' + message.destinationName + '  | ' + message.payloadString + '</span><br/>';
}

// Called when the disconnection button is pressed
function startDisconnect() {
    clkkmqtt.disconnect();
    document.getElementById("messages").innerHTML += '<span>Disconnected</span><br/>';
}

function Clearall(){
  
    document.getElementById("messages").innerHTML = "";
}

// Updates #messages div to auto-scroll
function updateScroll() {
    var element = document.getElementById("messages");
    element.scrollTop = element.scrollHeight;
}
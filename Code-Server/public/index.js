let data;
let marker;
let id_out ;


const mymap = L.map('Map_divice').setView([10.7822576,106.6503497], 12);
const attribution = '© <a href="http://osm.org/copyright">OpenStreetMap</a> contributors';
const title_url = 'http://{s}.tile.osm.org/{z}/{x}/{y}.png';
const title = L.tileLayer(title_url, {attribution} );
title.addTo(mymap);

//const marker = L.marker([0, 0]).addTo(mymap);



async function get_total_divice(){
    const ulr_listdivice = '/divice-connect';
    const response = await fetch(ulr_listdivice);
    data = await response.json();
   // console.log("data = ", data);
};


async function show_location(){
    await get_total_divice();
    let  j;
    for (j = 0; j < data.length; j++) {
        let id_iot =  data[j].id_device;
        marker = new L.marker([data[j].latitude, data[j].longitude])
          // .bindPopup('Id Divice: ' + data[j].id_device +'<a href="divice-chart.html"> View Chart</a>"').openPopup()
            .bindPopup(new L.popup().setContent(()=>{ //sessionStorage.setItem =  id_iot; 
                                                     let ulr =`<a href="divice-chart.html">Id Divice: ${id_iot}</a>`
                                                     return ulr;}))
           // .openPopup()
            .on('click', ()=>{localStorage.setItem("id_frominde", `${id_iot}`);})
            .addTo(mymap);

    }
}

show_location();


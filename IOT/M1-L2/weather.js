var Client = require('node-rest-client').Client;
var client = new Client();
// direct way
client.get("[URL]",function (data, response) {
    // parsed response body as js object
    //console.log(data);
    console.log(data.main.temp_max-273.15);
});


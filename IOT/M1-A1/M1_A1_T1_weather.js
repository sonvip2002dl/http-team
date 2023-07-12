/*************************************************
 *         © Keysight Technologies 2019 
 *
 * PROPRIETARY RIGHTS of Keysight Technologies are 
 * involved in the subject matter of this software. 
 * All manufacturing, reproduction, use, and sales 
 * rights pertaining to this software are governed 
 * by the license agreement. The recipient of this 
 * code implicitly accepts the terms of the license. 
 *
 ***************************************************
 *
 * FILE NAME  :  M1_A1_T1_weather.js      
 * DESCRIPTION:  This code reads the input from B2 
 *               and returns the corresponding value.
 * *************************************************/
 
var Client = require('node-rest-client').Client;
var client = new Client();
client.get("http://api.openweathermap.org/data/2.5/weather?q=Malaysia&APPID=be212827b49932f13120c3cc069b5573",function (data, response) {
    console.log(data.weather[0].description);
/*    require("child_process").exec('/home/debian/LabCode/M1-A1/weather '+ (data.weather[0].description.replace(" ","_")))*/
    require("child_process").exec('/home/debian/LabCode/M1-A1/weather '+ ("rain"))
});


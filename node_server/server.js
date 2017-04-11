var express = require('express');
var app = express();
var fs = require("fs");

app.get('/sensors', function(req, res) {
	fs.open(__dirname + "/" + "sensors.txt", 'r', function(status, fd) {
		var buffer = new Buffer(1024);
		fs.read(fd, buffer, 0, 1024, 0, function(err, num) {
			if(err) { console.log(err);};
			var sensor_data="";
			for(var i = 0; i<num; i+=24)
			{
				var SensorId = (buffer[i+1] << 8) | buffer[i];
				var Temperature = buffer[i+2];
				if(Temperature>127) {Temperature = buffer[i+2] - 256;};
				var Humidity = buffer[i+3];
				sensor_data += "Sensorin tunnus: " + SensorId + ", lämpötila: " + Temperature + "°C, kosteus: " + Humidity + "%\n"; 
				
				for(var k = 4; k<20; k++)
				{
					sensor_data += String.fromCharCode(parseInt(buffer[i+k], 1));
				}
			}
			console.log(sensor_data);
			res.end(sensor_data);
		});

	});
})

var server = app.listen(8001, function() {
	var host = server.address().address;
	var port = server.address().port;
	console.log("Example app listening on %s:%s", host, port);
})
var express = require('express');
var app = express();
var fs = require("fs");

app.get('/sensors', function(req, res) {
	fs.open(__dirname + "/" + "sensordata.txt", 'r', function(status, fd) {
		var buffer = new Buffer(1024);
		fs.read(fd, buffer, 0, 1024, 0, function(err, num) {
			if(err) { console.log(err);};
			var sensor_data="<html><table border=1><tr><th>Sensorin tunnus</th><th>L&#228;mp&#246;tila</th><th>Kosteus</th><th>Aikaleima</th></tr>";
			for(var i = 0; i<num; i+=24)
			{
				var SensorId = (buffer[i+1] << 8) | buffer[i];
				var Temperature = buffer[i+2];
				if(Temperature>127) {Temperature = buffer[i+2] - 256;};
				var Humidity = buffer[i+3];
				//sensor_data += "Sensorin tunnus: " + SensorId + ", l&#228;mp&#246;tila: " + Temperature + "C, kosteus: " + Humidity + "% "; 
				//var timedata=", Time: ";
				sensor_data += "<tr><td>" + SensorId + "</td><td>" + Temperature + "C</td><td>" + Humidity + "%</td><td>"; 	
				var timedata="";
				for(var k = 4; k<24; k++)
				{	
					if(buffer[i+k]!=0)
					{
						timedata += String.fromCharCode(buffer[i+k]);
					}
				}
				timedata+="</td></tr>"
				sensor_data += timedata; // + "<br>"
			}
			sensor_data += "</table></html>";
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
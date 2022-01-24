// Get current sensor readings when the page loads
window.addEventListener('load', getReadings);

// Create Temperature Chart
var chartT = new Highcharts.Chart({
  chart:{
    renderTo:'chart-temperature'
  },
  series: [
    {
      name: 'Temperature #1',
      type: 'line',
      color: '#101D42',
      marker: {
        symbol: 'circle',
        radius: 3,
        fillColor: '#101D42',
      }
    }
  ],
  title: {
    text: undefined
  },
  xAxis: {
    type: 'datetime',
    dateTimeLabelFormats: { second: '%H:%M:%S' }
  },
  yAxis: {
    title: {
      text: 'Temperature Celsius Degrees'
    }
  },
  credits: {
    enabled: false
  }
});

// Create Humidity Chart
var chartH = new Highcharts.Chart({
  chart:{
    renderTo:'chart-humidity'
  },
  series: [
    {
      name: 'Humidity #2',
      type: 'line',
      color: '#00A6A6',
      marker: {
        symbol: 'square',
        radius: 3,
        fillColor: '#00A6A6',
      }
    },
  ],
  title: {
    text: undefined
  },
  xAxis: {
    type: 'datetime',
    dateTimeLabelFormats: { second: '%H:%M:%S' }
  },
  yAxis: {
    title: {
      text: 'Humidity'
    }
  },
  credits: {
    enabled: false
  }
});

// Create Luminosity Chart
var chartL = new Highcharts.Chart({
  chart:{
    renderTo:'chart-luminosity'
  },
  series: [
    {
      name: 'Luminosity #3',
      type: 'line',
      color: '#8B2635',
      marker: {
        symbol: 'triangle',
        radius: 3,
        fillColor: '#8B2635',
      }
    },
  ],
  title: {
    text: undefined
  },
  xAxis: {
    type: 'datetime',
    dateTimeLabelFormats: { second: '%H:%M:%S' }
  },
  yAxis: {
    title: {
      text: 'Luminosity'
    }
  },
  credits: {
    enabled: false
  }
});

//TODO: az osszesa dat erkezik asszerint bonttani szet
//Plot temperature in the temperature chart
function plotData(jsonValue) {
	var time = (new Date()).getTime();
	
	var temperature = Number(jsonValue["temperature"]);
    console.log("temperature " + temperature);
	var humidity = Number(jsonValue["humidity"]);
    console.log("humidity " + humidity);
	var luminosity = Number(jsonValue["luminosity"]);
    console.log("humidity " + humidity);
	var manualswitch = Boolean(jsonValue["manualswitch"]);
    console.log("manualswitch " + manualswitch);
	var rtc_datetime = String(jsonValue["rtc_datetime"]);
    console.log("rtc_datetime " + rtc_datetime);

    if(chartT.series[0].data.length > 40) {
      chartT.series[0].addPoint([time, temperature], true, true, true);
	  chartH.series[0].addPoint([time, humidity], true, true, true);
	  chartL.series[0].addPoint([time, luminosity], true, true, true);
    } else {
      chartT.series[0].addPoint([time, temperature], true, false, true);
	  chartH.series[0].addPoint([time, humidity], true, false, true);
	  chartL.series[0].addPoint([time, luminosity], true, false, true);
    }
	
	document.getElementById("time").innerHTML = "Time: " + rtc_datetime;
	if (manualswitch) {
		document.getElementById("switch-state").innerHTML = "Switch state: ON";
	} else {
		document.getElementById("switch-state").innerHTML = "Switch state: OFF";
	}

}

// Function to get current readings on the webpage when it loads for the first time
function getReadings(){
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var myObj = JSON.parse(this.responseText);
      console.log(myObj);
      plotData(myObj);
    }
  };
  xhr.open("GET", "/readings", true);
  xhr.send();
}

if (!!window.EventSource) {
  var source = new EventSource('/events');

  source.addEventListener('open', function(e) {
    console.log("Events Connected");
  }, false);

  source.addEventListener('error', function(e) {
    if (e.target.readyState != EventSource.OPEN) {
      console.log("Events Disconnected");
    }
  }, false);

  source.addEventListener('message', function(e) {
    console.log("message", e.data);
  }, false);

  source.addEventListener('new_readings', function(e) {
    console.log("new_readings", e.data);
    var myObj = JSON.parse(e.data);
    console.log(myObj);
    plotData(myObj);
  }, false);
}
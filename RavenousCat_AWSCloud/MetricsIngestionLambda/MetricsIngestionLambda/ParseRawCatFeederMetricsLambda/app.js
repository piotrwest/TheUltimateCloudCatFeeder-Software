const
    stringify = require('csv-stringify-as-promised'),
    AWS = require('aws-sdk'),
    S3 = new AWS.S3();

/*global measureTypeMapping:true*/
measureTypeMapping = {
    "F": "FoodStorageLevels",
    "V": "VoltageLevels",
    "T": "Temp",
    "SL": "StepsPerLidOperation",
    "SF": "StepsPerFoodGram",
    "CTB": "CatToBowlTime",
    "FBO": "BeforeLidOpeningStats",
    "FA": "FoodAmount",
    "CW": "CatWeight",
    "ES": "EatingStats",
    "HS": "MinFreeHeapSize",
    "PLL": "ProtothreadLoopLength",
    "CD": "ClockDrift",
    "MBU": "MessageBufferUtilization",
    "MQTTS": "MqttPublishedSuccessfully",
    "MQTTE": "MqttPublishError",
    "AWSIOTS": "AWSIoTClientState",
    "HR": "HardwareReset",
    "WON": "WiFiConnected",
    "RS": "ReverseFoodMotorSteps",
    "AEF": "AfterEatingFoodAmount"
};

/*global csvColumns:true*/
csvColumns = [
    "timestamp",
	"type",
	"PLL_WifiConnectionManager",
	"PLL_WifiPublisher",
	"PLL_WifiKeepAliveAndReceive",
	"PLL_Buzzer",
	"PLL_FoodStorage",
	"PLL_VoltageSensors",
	"PLL_TempSensor",
	"PLL_LidMotor",
	"PLL_HealthMonitor",
	"PLL_LedRing",
	"PLL_FoodPortionDispenser",
	"PLL_NTPSync",
	"PLL_FoodPortionScheduler",
	"PLL_Total",
	"FSL_L1",
	"FSL_L2",
	"FSL_L3",
	"VoltageLevels_v1",
	"VoltageLevels_v2",
	"VoltageLevels_v3",
	"VoltageLevels_v4",
	"BeforeLidOpeningStats_actualFoodAmount",
	"BeforeLidOpeningStats_wantedFoodAmount",
	"EatingStats_eatingTimeInSeconds",
	"EatingStats_startFoodAmount",
	"EatingStats_endFoodAmount",
	"Temp",
	"FoodAmount",
	"CatWeight",
	"AfterEatingFoodAmount",
	"StepsPerLidOperation",
	"StepsPerFoodGram",
	"CatToBowlTime",
	"MinFreeHeapSize",
	"ClockDrift",
	"MessageBufferUtilization",
	"MqttPublishedSuccessfully",
	"MqttPublishError",
	"AWSIoTClientState",
	"HardwareReset",
	"WiFiConnected",
	"ReverseFoodMotorSteps",
];

function handleFloat(resultObj, measureValue) {
    resultObj["value"] = parseFloat(measureValue);
}

function handleInt(resultObj, measureValue) {
    resultObj["value"] = parseInt(measureValue);
}

function formatAndPushMeasure(measureTime, decodedMeasureType, measureValues, resultArray) {
    var resultObj = {};
    resultObj["timestamp"] = measureTime;
    resultObj["type"] = decodedMeasureType;
    try {
        switch (decodedMeasureType) {
            case "ProtothreadLoopLength":
                var values = measureValues.split('|');
                resultObj["WifiConnectionManager"] = parseInt(values[0]);
                resultObj["WifiPublisher"] = parseInt(values[1]);
                resultObj["WifiKeepAliveAndReceive"] = parseInt(values[2]);
                resultObj["Buzzer"] = parseInt(values[3]);
                resultObj["FoodStorage"] = parseInt(values[4]);
                resultObj["VoltageSensors"] = parseInt(values[5]);
                resultObj["TempSensor"] = parseInt(values[6]);
                resultObj["LidMotor"] = parseInt(values[7]);
                resultObj["HealthMonitor"] = parseInt(values[8]);
                resultObj["LedRing"] = parseInt(values[9]);
                resultObj["FoodPortionDispenser"] = parseInt(values[10]);
                resultObj["NTPSync"] = parseInt(values[11]);
                resultObj["FoodPortionScheduler"] = parseInt(values[12]);
                resultObj["Total"] = parseInt(values[13]);
                break;
            case "FoodStorageLevels":
                var values = measureValues.split('|');
                resultObj["level1"] = parseInt(values[0]);
                resultObj["level2"] = parseInt(values[1]);
                resultObj["level3"] = parseInt(values[2]);
                break;
            case "VoltageLevels":
                var values = measureValues.split('|');
                resultObj["v1"] = parseFloat(values[0]);
                resultObj["v2"] = parseFloat(values[1]);
                resultObj["v3"] = parseFloat(values[2]);
                resultObj["v4"] = parseFloat(values[3]);
                break;
            case "Temp":
            case "FoodAmount":
            case "CatWeight":
            case "AfterEatingFoodAmount":
                handleFloat(resultObj, measureValues);
                break;
            case "StepsPerLidOperation":
            case "StepsPerFoodGram":
            case "CatToBowlTime":
            case "MinFreeHeapSize":
            case "ClockDrift":
            case "MessageBufferUtilization":
            case "MqttPublishedSuccessfully":
            case "MqttPublishError":
            case "AWSIoTClientState":
            case "HardwareReset":
            case "WiFiConnected":
            case "ReverseFoodMotorSteps":
                handleInt(resultObj, measureValues);
                break;
            case "BeforeLidOpeningStats":
                var values = measureValues.split('|');
                resultObj["actualFoodAmount"] = parseFloat(values[0]);
                resultObj["wantedFoodAmount"] = parseFloat(values[1]);
                break;
            case "EatingStats":
                var values = measureValues.split('|');
                resultObj["eatingTimeInSeconds"] = parseFloat(values[0]);
                resultObj["startFoodAmount"] = parseFloat(values[1]);
                resultObj["endFoodAmount"] = parseFloat(values[2]);
                break;
            default:
                resultObj["rawValue"] = measureValues;
                break;
        }
    }
    catch (err) {
        console.error("ERROR occurred while parsing measure: " + err);
        resultObj["err"] = err;
    }

    resultArray.push(resultObj);
}

function convertJsonToCsv(jsonMetrics) {
	var renameProperty = function (obj, oldName, newName) {
		 // Do nothing if the names are the same
		 if (oldName === newName) {
			 return obj;
		 }
		// Check for the old property name to avoid a ReferenceError in strict mode.
		if (obj.hasOwnProperty(oldName)) {
			obj[newName] = obj[oldName];
			delete obj[oldName];
		}
		return obj;
	};

	jsonMetrics.forEach(element => {
		element.timestamp = element.timestamp.toISOString();
		switch (element.type) {
            case "ProtothreadLoopLength":
				renameProperty(element, "WifiConnectionManager", "PLL_WifiConnectionManager");
				renameProperty(element, "WifiPublisher", "PLL_WifiPublisher");
				renameProperty(element, "WifiKeepAliveAndReceive", "PLL_WifiKeepAliveAndReceive");
				renameProperty(element, "Buzzer", "PLL_Buzzer");
				renameProperty(element, "FoodStorage", "PLL_FoodStorage");
				renameProperty(element, "VoltageSensors", "PLL_VoltageSensors");
				renameProperty(element, "TempSensor", "PLL_TempSensor");
				renameProperty(element, "LidMotor", "PLL_LidMotor");
				renameProperty(element, "HealthMonitor", "PLL_HealthMonitor");
				renameProperty(element, "LedRing", "PLL_LedRing");
				renameProperty(element, "FoodPortionDispenser", "PLL_FoodPortionDispenser");
				renameProperty(element, "NTPSync", "PLL_NTPSync");
				renameProperty(element, "FoodPortionScheduler", "PLL_FoodPortionScheduler");
				renameProperty(element, "Total", "PLL_Total");
                break;
            case "FoodStorageLevels":
				renameProperty(element, "level1", "FSL_L1");
				renameProperty(element, "level2", "FSL_L2");
				renameProperty(element, "level3", "FSL_L3");
                break;
            case "VoltageLevels":
				renameProperty(element, "v1", "VoltageLevels_v1");
				renameProperty(element, "v2", "VoltageLevels_v2");
				renameProperty(element, "v3", "VoltageLevels_v3");
				renameProperty(element, "v4", "VoltageLevels_v4");
                break;
            case "Temp":
            case "FoodAmount":
            case "CatWeight":
            case "AfterEatingFoodAmount":
            case "StepsPerLidOperation":
            case "StepsPerFoodGram":
            case "CatToBowlTime":
            case "MinFreeHeapSize":
            case "ClockDrift":
            case "MessageBufferUtilization":
            case "MqttPublishedSuccessfully":
            case "MqttPublishError":
            case "AWSIoTClientState":
            case "HardwareReset":
            case "WiFiConnected":
            case "ReverseFoodMotorSteps":
                renameProperty(element, "value", element.type);
                break;
            case "BeforeLidOpeningStats":
                renameProperty(element, "actualFoodAmount", "BeforeLidOpeningStats_actualFoodAmount");
				renameProperty(element, "wantedFoodAmount", "BeforeLidOpeningStats_wantedFoodAmount");
                break;
            case "EatingStats":
				renameProperty(element, "eatingTimeInSeconds", "EatingStats_eatingTimeInSeconds");
				renameProperty(element, "startFoodAmount", "EatingStats_startFoodAmount");
				renameProperty(element, "endFoodAmount", "EatingStats_endFoodAmount");
                break;
            default:
				throw new Error("FAIL TO DECODE TO CSV!");
        }
	});
	var options = { header: true, columns: csvColumns };
	
	return stringify(jsonMetrics, options);
}

/**
 *
 * Event doc: https://docs.aws.amazon.com/apigateway/latest/developerguide/set-up-lambda-proxy-integrations.html#api-gateway-simple-proxy-for-lambda-input-format
 * @param {Object} event - from AWS IoT
 *
 * Context doc: https://docs.aws.amazon.com/lambda/latest/dg/nodejs-prog-model-context.html 
 * @param {Object} context
 *
 * Return doc: https://docs.aws.amazon.com/apigateway/latest/developerguide/set-up-lambda-proxy-integrations.html
 * @returns {Object} object - API Gateway Lambda Proxy Output Format
 * 
 */
exports.lambdaHandler = async (event, context, callback) => {
    var encodedData = event.payload;
    var decodedData = Buffer.from(encodedData, 'base64').toString();
    console.log("Received: " + decodedData);

    var measures = decodedData.split('#');
    var rootTimeAsDeciseconds = measures[0].split('@')[1];
    var rootTime = new Date(rootTimeAsDeciseconds * 100);
    console.log("Root time: " + rootTime);

    var result = [];

    for (var i = 1; i < measures.length; i++) {
        if (measures[i]) {
            var splitted = measures[i].split('@');
            var measureType = splitted[0];
            var diffMillis = splitted[1] * 100;
            var measureTime = new Date(rootTime.getTime() + diffMillis);
            var measureValues = splitted[2];
            //console.log("At: " + measureTime + " measure: " + measureType + " VALS: " + measureValues);

            if (!(measureType in measureTypeMapping)) {
                console.warn("UNKNOWN MEASURE TYPE!!: " + measureType);
                continue;
            }

            formatAndPushMeasure(measureTime, measureTypeMapping[measureType], measureValues, result);
        }
    }
    
    var data_MessageBufferUtilization = result.filter(x => x.type == "MessageBufferUtilization");
    var data_MqttPublishedSuccessfully = result.filter(x => x.type == "MqttPublishedSuccessfully");
    var data_MqttPublishError = result.filter(x => x.type == "MqttPublishError");
    result = result.filter(x => x.type != "MessageBufferUtilization" && x.type != "MqttPublishedSuccessfully" &&  x.type != "MqttPublishError");
    
    if (data_MessageBufferUtilization.length) {
        data_MessageBufferUtilization = data_MessageBufferUtilization.reduce(function (prev, curr) {
            return prev.value > curr.value ? prev : curr;
        });
        result.push(data_MessageBufferUtilization); 
    }
    
    if (data_MqttPublishedSuccessfully.length) {
        data_MqttPublishedSuccessfully = data_MqttPublishedSuccessfully.reduce(function (prev, curr) {
            return prev.value > curr.value ? prev : curr;
        });
        result.push(data_MqttPublishedSuccessfully);
    }
    
    if (data_MqttPublishError.length) {
        data_MqttPublishError = data_MqttPublishError.reduce(function (prev, curr) {
            return prev.value > curr.value ? prev : curr;
        });
        result.push(data_MqttPublishError);   
    }

    var jsonToUpload = JSON.stringify(result);
    console.log("Produced measures: " + jsonToUpload);
    
    //this: rootTime.toISOString().split('T')[0];
    //gets: "2016-02-18"
    //then after replace it's something like "2016/02/18"
    var yearMonthDay = rootTime.toISOString().replace('-', '/').split('T')[0].replace('-', '/');

    var uploadJsonData = function() {
        var key = 'data' + "/" + yearMonthDay;
        key = key + "/" + rootTime.toISOString() + ".json";
        console.log("Will upload JSON to: " + key);
    
        var params = {
            Bucket: "catfeederdata",
            Key: key,
            Body: jsonToUpload,
            CacheControl: 'no-cache',
            ContentType: "application/json"
        };
        
        return S3.putObject(params).promise();
    }
    
    var uploadRawData = function() {
        var key = "data_raw" + "/" + yearMonthDay + "/" + rootTimeAsDeciseconds + ".txt";
        console.log("Will upload raw to: " + key)
        var params = {
            Bucket: "catfeederdata",
            Key: key,
            Body: decodedData,
            CacheControl: 'no-cache',
            ContentType: "text/plain"
        };
        
        return S3.putObject(params).promise();
    }
    
    var uploadCsvData = function() {
        return convertJsonToCsv(result).then(function(csvData) {
           var key = "data_csv" + "/" + yearMonthDay + "/" + rootTimeAsDeciseconds + ".csv";
            console.log("Will upload csv to: " + key)
            var params = {
                Bucket: "catfeederdata",
                Key: key,
                Body: csvData,
                CacheControl: 'no-cache',
                ContentType: "text/csv"
            };
            
            return S3.putObject(params).promise();
        });
    }
    
    return Promise.all([uploadJsonData(), uploadRawData(), uploadCsvData()]);
};

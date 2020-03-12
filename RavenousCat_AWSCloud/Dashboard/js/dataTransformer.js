(function dataTransformer() {
    var MAX_POINTS_TO_DISPLAY = 200;
    var maxCallback = ( max, cur ) => Math.max( max, cur );
    var minCallback = ( min, cur ) => Math.min( min, cur );
    var PROTOTHREAD_NAMES = [
        "WifiConnectionManager",
        "WifiPublisher",
        "WifiKeepAliveAndReceive",
        "Buzzer",
        "FoodStorage",
        "VoltageSensors",
        "TempSensor",
        "LidMotor",
        "HealthMonitor",
        "LedRing",
        "FoodPortionDispenser",
        "NTPSync",
        "FoodPortionScheduler",
        "Total"];

    function linearGatherIfCloserThanTreshold(data, secondsThreshold) {
        if (data.length == 0) {
            return;
        }
        var result = [];
        var previousItem = data[0];
        var lastBucket = [];
        lastBucket.push(previousItem);
        result.push(lastBucket);
    
        for (var i = 1; i < data.length; i++) {
            var currentItem = data[i];
            if (Math.abs(previousItem.timestamp.diff(currentItem.timestamp, 'seconds')) < secondsThreshold) {
                lastBucket.push(currentItem);
            } else {
                lastBucket = [];
                lastBucket.push(currentItem);
                result.push(lastBucket);
            }
            previousItem = currentItem;
        }
        return result;
    }

    function linearBucketize(data, secondsThreshold) {
        function keyFromTimestamp(t) {
            return t.valueOf() - (t.valueOf() % (secondsThreshold*1000));
        }
        
        var result = [];

        if (data == null || data.length == 0) {
            return result;
        }
        
        var previousItem = data[0];
        var lastBucket = [];
        lastBucket.push(previousItem);
        result.push(lastBucket);
    
        for (var i = 1; i < data.length; i++) {
            var currentItem = data[i];
            if (keyFromTimestamp(previousItem.timestamp) == keyFromTimestamp(currentItem.timestamp)) {
                lastBucket.push(currentItem);
            } else {
                lastBucket = [];
                lastBucket.push(currentItem);
                result.push(lastBucket);
            }
            previousItem = currentItem;
        }
        return result;
    }

    function groupBy(list, keyGetter) {
        const map = new Map();
        list.forEach((item) => {
             const key = keyGetter(item);
             const collection = map.get(key);
             if (!collection) {
                 map.set(key, [item]);
             } else {
                 collection.push(item);
             }
        });
        return map;
    }

    window.DataTransformer = {};
    window.DataTransformer.PrepareData = function PrepareData(s3Data) {
        var result = {};
        if (s3Data.length == 0) {
            return result;
        }
        s3Data = s3Data.flatMap(d => d); //aggregate from multiple files
        s3Data.forEach(data => {
            data.timestamp = moment(data.timestamp);
        });
        s3Data.sort(function cmp(a, b) { return a.timestamp.valueOf() - b.timestamp.valueOf(); });
        result.minDate = s3Data[0].timestamp;
        result.maxDate = s3Data[s3Data.length - 1].timestamp;

        grouppedRawData = groupBy(s3Data, i => i.type);
        result.grouppedData = {};
        result.grouppedData.CatWeight = prepareCatWeight(grouppedRawData.get("CatWeight"));

        assignEmptyIfUndefined(result.grouppedData, grouppedRawData, "BeforeLidOpeningStats");
        assignEmptyIfUndefined(result.grouppedData, grouppedRawData, "EatingStats");
        assignEmptyIfUndefined(result.grouppedData, grouppedRawData, "CatToBowlTime");
        assignEmptyIfUndefined(result.grouppedData, grouppedRawData, "StepsPerLidOperation");
        assignEmptyIfUndefined(result.grouppedData, grouppedRawData, "StepsPerFoodGram");
        assignEmptyIfUndefined(result.grouppedData, grouppedRawData, "ClockDrift");
        assignEmptyIfUndefined(result.grouppedData, grouppedRawData, "MqttPublishedSuccessfully");
        assignEmptyIfUndefined(result.grouppedData, grouppedRawData, "MqttPublishError");
        assignEmptyIfUndefined(result.grouppedData, grouppedRawData, "HardwareReset");
        assignEmptyIfUndefined(result.grouppedData, grouppedRawData, "WiFiConnected");
        assignEmptyIfUndefined(result.grouppedData, grouppedRawData, "ReverseFoodMotorSteps");
        assignEmptyIfUndefined(result.grouppedData, grouppedRawData, "AfterEatingFoodAmount");

        //1 min metric preparation:
        var timeRangeInSeconds = Math.abs(result.minDate.diff(result.maxDate, 'seconds'));
        var bucketSizeInSeconds = Math.round(Math.max(1, timeRangeInSeconds / MAX_POINTS_TO_DISPLAY));

        result.grouppedData.Temp = linearBucketize(grouppedRawData.get("Temp"), bucketSizeInSeconds).flatMap(bucket => {
            var avg = bucket.map(x => x.value).reduce((acc, curr) => acc + curr) / bucket.length;
            return { timestamp: bucket[0].timestamp, type: bucket[0].type, value: avg };
        });

        result.grouppedData.MinFreeHeapSize = linearBucketize(grouppedRawData.get("MinFreeHeapSize"), bucketSizeInSeconds).flatMap(bucket => {
            var min = bucket.map(x => x.value).reduce(minCallback);
            return { timestamp: bucket[0].timestamp, type: bucket[0].type, value: min };
        });

        result.grouppedData.MessageBufferUtilization = linearBucketize(grouppedRawData.get("MessageBufferUtilization"), bucketSizeInSeconds).flatMap(bucket => {
            var avg = bucket.map(x => x.value).reduce((acc, curr) => acc + curr) / bucket.length;
            return { timestamp: bucket[0].timestamp, type: bucket[0].type, value: avg };
        });

        result.grouppedData.ProtothreadLoopLength = [];
        result.grouppedData.ProtothreadLoopLength.Timestamps = [];
        result.grouppedData.ProtothreadLoopLength.Types = {};
        PROTOTHREAD_NAMES.forEach(function(pName) {
            result.grouppedData.ProtothreadLoopLength.Types[pName] = [];
        });
        linearBucketize(grouppedRawData.get("ProtothreadLoopLength"), bucketSizeInSeconds).forEach(function(bucket){
            result.grouppedData.ProtothreadLoopLength.Timestamps.push(bucket[0].timestamp);
            PROTOTHREAD_NAMES.forEach(function(pName) {
                result.grouppedData.ProtothreadLoopLength.Types[pName].push(bucket.map(x => x[pName]).reduce(maxCallback));
            });
        });
        return result;
    }

    function assignEmptyIfUndefined(grouppedData, grouppedRawData, propName) {
        if (grouppedRawData.get(propName)) {
            grouppedData[propName] = grouppedRawData.get(propName);
        } else {
            grouppedData[propName] = [];
        }
    }

    function prepareCatWeight(data_catWeight) {
        if (!data_catWeight) {
            return [];
        }
        function med(values) {
            if (values.length === 0) return 0
        
            values.sort(function (a, b) {
                return a - b;
            });
        
        
            var half = Math.floor(values.length / 2);
        
            if (values.length % 2)
                return values[half];
            else
                return (values[half - 1] + values[half]) / 2.0;
        }

        data_catWeight = linearGatherIfCloserThanTreshold(data_catWeight, 15 * 60);
        data_catWeight = data_catWeight.flatMap(bucket => {
            bucket = bucket.filter(x => x.value > CatFeederConfig.CatMinWeight && x.value < CatFeederConfig.CatMaxWeight);
            if (bucket.length > 0) {
                var catWeights = bucket.map(weightWithTimestamp => weightWithTimestamp.value);
                var median = med(catWeights);
                return { timestamp: bucket[0].timestamp, type: bucket[0].type, value: median };
            } else {
                return null;
            }
        }).filter(reading => reading != null);
        return data_catWeight;
    }

})();
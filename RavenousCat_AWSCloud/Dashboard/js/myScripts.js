var graphData_data_s3_Files = [];

function drawCatWeight(data_catWeight) {
    var graphData_data_catWeight_Values = data_catWeight.map(cw => cw.value);
    var graphData_data_catWeight_Time = data_catWeight.map(cw => cw.timestamp.format('MMM DD HH:mm'));

    catWeightLineEChartGraph.setOption({
        tooltip: {
            trigger: 'axis',
            formatter: function (params) {
                params = params[0];
                return params.name + ' -> ' + params.value + 'g';
            },
            axisPointer: {
                animation: false
            }
        },
        xAxis: [{
            type: 'category',
            data: graphData_data_catWeight_Time,
            boundaryGap: false,
            splitLine: {
                show: false
            }
        }],
        yAxis: [{
            type: 'value',
            scale: true,
            boundaryGap: [0.2, 0.2],
            splitLine: {
                show: false
            },
            axisLabel: {
                formatter: '{value} g'
            }
        }],
        series: [{
            name: 'cat weight',
            type: 'line',
            data: graphData_data_catWeight_Values
        }]
    });
}

function drawBeforeLidOpeningStats(data_beforeLidOpeningStats) {
    var graphData_data_beforeLidOpeningStats_Time = data_beforeLidOpeningStats.map(x => x.timestamp.format('MMM DD HH:mm'));
    var graphData_data_beforeLidOpeningStats_BaseAmount = [];
    var graphData_data_beforeLidOpeningStats_MissingAmount = [];
    var graphData_data_beforeLidOpeningStats_ExtraAmount = [];
    data_beforeLidOpeningStats.forEach(stats => {
        if (stats.wantedFoodAmount == stats.actualFoodAmount) {
            graphData_data_beforeLidOpeningStats_BaseAmount.push(stats.wantedFoodAmount.toFixed(2));
            graphData_data_beforeLidOpeningStats_MissingAmount.push(parseFloat(0).toFixed(2));
            graphData_data_beforeLidOpeningStats_ExtraAmount.push(parseFloat(0).toFixed(2));
        } else if (stats.wantedFoodAmount > stats.actualFoodAmount) { //given not enough food
            graphData_data_beforeLidOpeningStats_BaseAmount.push(stats.actualFoodAmount.toFixed(2));
            graphData_data_beforeLidOpeningStats_MissingAmount.push((stats.wantedFoodAmount - stats.actualFoodAmount).toFixed(2));
            graphData_data_beforeLidOpeningStats_ExtraAmount.push(parseFloat(0).toFixed(2));
        } else if (stats.wantedFoodAmount < stats.actualFoodAmount) { //given not enough food
            graphData_data_beforeLidOpeningStats_BaseAmount.push(stats.wantedFoodAmount.toFixed(2));
            graphData_data_beforeLidOpeningStats_MissingAmount.push(parseFloat(0).toFixed(2));
            graphData_data_beforeLidOpeningStats_ExtraAmount.push((stats.actualFoodAmount - stats.wantedFoodAmount).toFixed(2));
        }
    });

    foodGivenLineEChartGraph.setOption({
        tooltip: {
            trigger: 'axis'
        },
        legend: {
            data: ['Base', 'Missing', "Extra"],
            align: 'left',
            left: 10
        },
        xAxis: [{
            type: 'category',
            data: graphData_data_beforeLidOpeningStats_Time,
            boundaryGap: true,
            splitLine: {
                show: false
            }
        }],
        yAxis: [{
            boundaryGap: [0, 0.2],
            splitLine: {
                show: false
            },
            axisLabel: {
                formatter: '{value} g'
            }
        }],
        series: [{
            name: 'Base',
            type: 'bar',
            stack: 'one',
            data: graphData_data_beforeLidOpeningStats_BaseAmount
        }, {
            name: 'Missing',
            type: 'bar',
            stack: 'one',
            data: graphData_data_beforeLidOpeningStats_MissingAmount
        }, {
            name: 'Extra',
            type: 'bar',
            stack: 'one',
            data: graphData_data_beforeLidOpeningStats_ExtraAmount
        }
        ]
    });
}

function drawEatingStats(data_eatingStats, minDate, maxDate) {
    var xAxis = [];
    var currentDate = moment(minDate);
    currentDate.startOf('hour');

    //push every hour marker between minDate and maxDate
    while (currentDate.isBefore(maxDate)) {
        xAxis.push(moment(currentDate.toISOString()));
        currentDate = currentDate.add(1, 'hours');
    }

    var graphData_eatingStatsByHour_foodEaten = [];
    var graphData_eatingStatsByHour_eatingSpeed = [];
    for (var i = 0; i < xAxis.length; i++) {
        var axisTime = xAxis[i];

        var thisHourEatingStats = data_eatingStats.filter(x => {
            time = x.timestamp;
            time.startOf('hour');
            return time.isSame(axisTime);
        });

        var sumInGrams = thisHourEatingStats.reduce(function (acc, currentVal) {
            return acc + (currentVal.startFoodAmount - currentVal.endFoodAmount);
        }, 0);
        var totalTimeInSeconds = thisHourEatingStats.reduce(function (acc, currentVal) {
            return acc + currentVal.eatingTimeInSeconds;
        }, 0);

        graphData_eatingStatsByHour_foodEaten.push(sumInGrams);
        if (sumInGrams > 0 && (sumInGrams / totalTimeInSeconds) > 0.00001) {
            graphData_eatingStatsByHour_eatingSpeed.push(sumInGrams / totalTimeInSeconds);
        } else {
            graphData_eatingStatsByHour_eatingSpeed.push(null);
        }
    }

    //xAxis is category, so change to string
    xAxis = xAxis.map(date => date.format('MMM DD HH:mm'));
    eatingSpeedChartGraph.setOption({
        tooltip: {
            trigger: 'axis',
            formatter: function (params) {
                var txt = '@ ' + params[0].name;
                txt += '</br>';
                if (params[0].value > 0) {
                    txt += "Food eaten: " + params[0].value.toFixed(2) + ' g';

                    if (params[1].value) {
                        txt += '</br>';
                        txt += "Consumption speed: " + params[1].value.toFixed(4) + ' g/s';
                    }
                } else {
                    txt += "Didn't consume any food...";
                }

                return txt;
            },
            axisPointer: {
                animation: false
            }
        },
        xAxis: [{
            type: 'category',
            boundaryGap: false,
            splitLine: {
                show: false
            },
            data: xAxis
        }],
        yAxis: [{
            type: 'value',
            boundaryGap: [0, 0.2],
            splitLine: {
                show: false
            },
            axisLabel: {
                formatter: '{value} g'
            }
        }, {
            type: 'value',
            boundaryGap: [0, 0.2],
            splitLine: {
                show: false
            },
            splitArea: {
                show: false
            },
            axisLabel: {
                formatter: '{value} g/s'
            }
        }],
        series: [{
            name: 'food eaten',
            type: 'bar',
            data: graphData_eatingStatsByHour_foodEaten
        }, {
            name: 'eating speed',
            type: 'bar',
            yAxisIndex: 1,
            data: graphData_eatingStatsByHour_eatingSpeed
        }]
    });
}

function drawCatToBowlTime(data_catToBowlTime) {
    var skippedPoints = [];
    graphData_data_catToBowlTime_Values = data_catToBowlTime.map(x => Math.max(0, x.value));
    graphData_data_catToBowlTime_Time = data_catToBowlTime.map(x => x.timestamp.format('MMM DD HH:mm'));

    for(var i = 0; i < data_catToBowlTime.length; i++) {
        if (data_catToBowlTime[i].value < 0) {
            skippedPoints.push({
                xAxis: i,
                yAxis: 0
            });
        }
    }

    catToBowlTimeChartGraph.setOption({
        tooltip: {
            trigger: 'axis',
            formatter: function (params) {
                params = params[0];
                if (params) {
                    if (params.value == 0) {
                        return "Cat didn't arrive within 30min @" + params.name;
                    }
                    return params.value + 's @' + params.name;
                }
            },
            axisPointer: {
                animation: false
            }
        },
        xAxis: [{
            type: 'category',
            data: graphData_data_catToBowlTime_Time,
            splitLine: {
                show: false
            }
        }],
        yAxis: [{
            type: 'value',
            splitLine: {
                show: false
            },
            axisLabel: {
                formatter: '{value} s'
            }
        }],
        series: [{
            name: 'Cat To Bowl Time',
            type: 'bar',
            data: graphData_data_catToBowlTime_Values,
            markPoint : {
                label: {
                    show: true,
                    normal: {
                        formatter: function (param) {
                            return "skipped!";
                        },
                        // textStyle: {
                        //     color: '#26B99A'
                        // },
                    }
                },
                data : skippedPoints
            }
        }]
    });
}

function drawReverseFoodMotorSteps(data_reverseFoodMotorSteps) {
    graphData_data_reverseFoodMotorSteps_Values = data_reverseFoodMotorSteps.map(x => x.value);
    graphData_data_reverseFoodMotorSteps_Time = data_reverseFoodMotorSteps.map(x => x.timestamp.format('MMM DD HH:mm'));

    reverseFoodMotorSteps.setOption({
        tooltip: {
            trigger: 'axis',
            formatter: function (params) {
                params = params[0];
                return params.value + ' steps @' + params.name;
            },
            axisPointer: {
                animation: false
            }
        },
        xAxis: [{
            type: 'category',
            data: graphData_data_reverseFoodMotorSteps_Time,
            splitLine: {
                show: false
            }
        }],
        yAxis: [{
            type: 'value',
            splitLine: {
                show: false
            }
        }],
        series: [{
            name: 'Reverse Food Motor Steps',
            type: 'bar',
            data: graphData_data_reverseFoodMotorSteps_Values
        }]
    });
}

function drawAfterEatingFoodAmount(data_afterEatingFoodAmount) {
    graphData_data_afterEatingFoodAmount_Values = data_afterEatingFoodAmount.map(x => x.value);
    graphData_data_afterEatingFoodAmount_Time = data_afterEatingFoodAmount.map(x => x.timestamp.format('MMM DD HH:mm'));

    afterEatingFoodAmount.setOption({
        tooltip: {
            trigger: 'axis',
            formatter: function (params) {
                params = params[0];
                return params.value + 'g @' + params.name;
            },
            axisPointer: {
                animation: false
            }
        },
        xAxis: [{
            type: 'category',
            data: graphData_data_afterEatingFoodAmount_Time,
            splitLine: {
                show: false
            }
        }],
        yAxis: [{
            type: 'value',
            splitLine: {
                show: false
            }
        }],
        series: [{
            name: 'Food Amount Reading After Eating',
            type: 'bar',
            data: graphData_data_afterEatingFoodAmount_Values
        }]
    });
}

function drawStepsPerLidOperation(data_stepsPerLidOperation) {
    var graphData_data_stepsPerLidOperation_Values = data_stepsPerLidOperation.map(cw => cw.value);
    var graphData_data_stepsPerLidOperation_Time = data_stepsPerLidOperation.map(cw => cw.timestamp.format('MMM DD HH:mm'));

    stepsPerLidOperationGraph.setOption({
        tooltip: {
            trigger: 'axis',
            formatter: function (params) {
                params = params[0];
                return params.value + ' steps @ ' + params.name;
            },
            axisPointer: {
                animation: false
            }
        },
        xAxis: [{
            type: 'category',
            data: graphData_data_stepsPerLidOperation_Time,
            boundaryGap: false,
            splitLine: {
                show: false
            }
        }],
        yAxis: [{
            type: 'value',
            scale: true,
            boundaryGap: [0.2, 0.2],
            splitLine: {
                show: false
            }
        }],
        series: [{
            name: 'motor steps',
            type: 'line',
            data: graphData_data_stepsPerLidOperation_Values
        }]
    });
}

function drawStepsPerFoodGram(data_stepsPerFoodGram) {
    var graphData_data_stepsPerFoodGram_Values = data_stepsPerFoodGram.map(cw => cw.value);
    var graphData_data_stepsPerFoodGram_Time = data_stepsPerFoodGram.map(cw => cw.timestamp.format('MMM DD HH:mm'));

    stepsPerFoodGramGraph.setOption({
        tooltip: {
            trigger: 'axis',
            formatter: function (params) {
                params = params[0];
                return params.value + ' steps @ ' + params.name;
            },
            axisPointer: {
                animation: false
            }
        },
        xAxis: [{
            type: 'category',
            data: graphData_data_stepsPerFoodGram_Time,
            boundaryGap: false,
            splitLine: {
                show: false
            }
        }],
        yAxis: [{
            type: 'value',
            splitLine: {
                show: false
            }
        }],
        series: [{
            name: 'motor steps',
            type: 'line',
            data: graphData_data_stepsPerFoodGram_Values
        }]
    });
}

function drawTemperatureGraph(data_temperature) {
    var graphData_data_temperature_Values = data_temperature.map(cw => cw.value);
    var graphData_data_temperature_Time = data_temperature.map(cw => cw.timestamp.format('MMM DD HH:mm'));

    temperatureGraph.setOption({
        tooltip: {
            trigger: 'axis',
            formatter: function (params) {
                params = params[0];
                return params.value.toFixed(2) + '°C @ ' + params.name;
            },
            axisPointer: {
                animation: false
            }
        },
        xAxis: [{
            type: 'category',
            data: graphData_data_temperature_Time,
            boundaryGap: false,
            splitLine: {
                show: false
            }
        }],
        yAxis: [{
            type: 'value',
            scale: true,
            splitLine: {
                show: false
            },
            axisLabel: {
                formatter: function (value, index) {
                    return value.toFixed(1);
                }
            }
        }],
        series: [{
            type: 'line',
            data: graphData_data_temperature_Values
        }]
    });
}

function drawFreeHeapSizeGraph(data_minFreeHeapSize) {
    var graphData_data_minFreeHeapSize_Values = data_minFreeHeapSize.map(cw => cw.value);
    var graphData_data_minFreeHeapSize_Time = data_minFreeHeapSize.map(cw => cw.timestamp.format('MMM DD HH:mm'));

    freeHeapSizeGraph.setOption({
        tooltip: {
            trigger: 'axis',
            formatter: function (params) {
                params = params[0];
                return params.value + ' bytes @ ' + params.name;
            },
            axisPointer: {
                animation: false
            }
        },
        xAxis: [{
            type: 'category',
            data: graphData_data_minFreeHeapSize_Time,
            boundaryGap: true,
            splitLine: {
                show: false
            }
        }],
        yAxis: [{
            type: 'value',
            splitLine: {
                show: false
            },
            //min: 51200,
            axisLabel: {
                formatter: function (value, index) {
                    return (value / 1024).toFixed(0);
                }
            }
        }],
        series: [{
            type: 'line',
            data: graphData_data_minFreeHeapSize_Values
        }]
    });
}

function drawMaxProtothreadLoopLengthGraphs(protothreadMetrics) {
    var longSeries = [];
    var quickSeries = [];
    for(var metricType in protothreadMetrics.Types) {
        var s = {};
        s.type = 'line';
        s.data = protothreadMetrics.Types[metricType];
        s.name = metricType;
        if (metricType == "Total" || metricType == "WifiPublisher" || metricType == "NTPSync") {
            longSeries.push(s);
        } else {
            quickSeries.push(s);
        }
    }

    var graphData_data_maxProtothreadLoopLength_Time = protothreadMetrics.Timestamps.map(t => t.format('MMM DD HH:mm'));

    function setGraphOptions(graph, seriesData) {
        graph.setOption({
            tooltip: {
                trigger: 'axis',
                formatter: function (params) {
                    var result = "";
                    result += "@ " + params[0].name + "</br>";
    
                    params.forEach(function (p) {
                        result += p.seriesName + ": " + (p.value / 1000.0).toFixed(0) + "ms</br>";
                    });
                    return result;
                },
                axisPointer: {
                    animation: false
                }
            },
            xAxis: [{
                type: 'category',
                data: graphData_data_maxProtothreadLoopLength_Time,
                boundaryGap: true,
                splitLine: {
                    show: false
                }
            }],
            yAxis: [{
                type: 'value',
                splitLine: {
                    show: false
                },
                //min: 51200,
                //max: 200000,
                axisLabel: {
                    formatter: function (value, index) {
                        return (value / 1000.0).toFixed(0) + " ms";
                    }
                }
            }],
            series: seriesData
        });
    }
    
    setGraphOptions(maxProtothreadLoopLengthGraphLong, longSeries);
    setGraphOptions(maxProtothreadLoopLengthGraphQuick, quickSeries);
}

function drawMessageBufferSizeGraph(data_messageBufferUtilization) {
    var graphData_data_messageBufferUtilization_Values = data_messageBufferUtilization.map(cw => cw.value);
    var graphData_data_messageBufferUtilization_Time = data_messageBufferUtilization.map(cw => cw.timestamp.format('MMM DD HH:mm'));

    messageSizeGraph.setOption({
        tooltip: {
            trigger: 'axis',
            formatter: function (params) {
                params = params[0];
                return params.value + ' bytes @ ' + params.name;
            },
            axisPointer: {
                animation: false
            }
        },
        xAxis: [{
            type: 'category',
            data: graphData_data_messageBufferUtilization_Time,
            boundaryGap: true,
            splitLine: {
                show: false
            }
        }],
        yAxis: [{
            type: 'value',
            splitLine: {
                show: false
            },
            max: 5000,
            axisLabel: {
                formatter: function (value, index) {
                    return (value / 1024).toFixed(0) + " kiB";
                }
            }
        }],
        series: [{
            type: 'line',
            data: graphData_data_messageBufferUtilization_Values
        }]
    });
}

function drawClockDriftGraph(data_clockDriftUs) {
    var graphData_data_clockDriftUs_Values = data_clockDriftUs.map(cw => cw.value);
    var graphData_data_clockDriftUs_Time = data_clockDriftUs.map(cw => cw.timestamp.format('MMM DD HH:mm'));

    clockDriftGraph.setOption({
        tooltip: {
            trigger: 'axis',
            formatter: function (params) {
                params = params[0];
                return params.value + ' μs @ ' + params.name;
            },
            axisPointer: {
                animation: false
            }
        },
        xAxis: [{
            type: 'category',
            data: graphData_data_clockDriftUs_Time,
            boundaryGap: true,
            splitLine: {
                show: false
            }
        }],
        yAxis: [{
            type: 'value',
            splitLine: {
                show: false
            },
            //min: 51200,
            axisLabel: {
                formatter: function (value, index) {
                    return (value / 100000).toFixed(1) + "s";
                }
            }
        }],
        series: [{
            type: 'line',
            data: graphData_data_clockDriftUs_Values
        }]
    });
}

function storeWrittenS3FilesGraphData(s3Keys) {
    graphData_data_s3_Files = s3Keys.map(s3ListObject => {
        var time = moment(s3ListObject.LastModified);
        time.startOf('hour');
        return {
            t: time,
            size: s3ListObject.Size
        };
    });
}

function getBaseLog(base, y) {
    return Math.log(y) / Math.log(base);
}

function drawConnectivityGraph(data_mqttSuccess, data_mqttError, data_hardwareReset, data_wiFiConnected, minDate, maxDate) {
    var xAxis = [];
    var currentDate = moment(minDate);
    currentDate.startOf('hour');

    //push every hour marker between minDate and maxDate
    while (currentDate.isBefore(maxDate)) {
        xAxis.push(moment(currentDate.toISOString()));
        currentDate = currentDate.add(1, 'hours');
    }

    //indicators are multiple X axis, on top of each other
    var indicators = ['S3 File Count', 'S3 File Sizes', 'MQTT Pub OK', 'MQTT Pub Fail', 'Hardware Resets', 'Offline'];
    var INDEX_S3_FILE_COUNT = 0;
    var INDEX_S3_FILE_SIZE = 1;
    var INDEX_MQTT_OK = 2;
    var INDEX_MQTT_FAIL = 3;
    var INDEX_HARDWARE_RESET = 4;
    var INDEX_OFFLINE = 5;
    //data scheme: array: [indicator, xAxis index, size]
    var data = [];

    //MQTT, HardwareReset and Online stats processing
    for (var i = 0; i < xAxis.length; i++) {
        axisTime = xAxis[i];

        var thisHourMQTTSuccess = data_mqttSuccess.filter(x => {
            time = x.timestamp;
            time.startOf('hour');
            return time.isSame(axisTime);
        });
        data.push([INDEX_MQTT_OK, i, thisHourMQTTSuccess.length]);

        var thisHourMQTTFail = data_mqttError.filter(x => {
            time = x.timestamp;
            time.startOf('hour');
            return time.isSame(axisTime);
        });
        data.push([INDEX_MQTT_FAIL, i, thisHourMQTTFail.length]);

        var thisHourhardwareReset = data_hardwareReset.filter(x => {
            time = x.timestamp;
            time.startOf('hour');
            return time.isSame(axisTime);
        });
        data.push([INDEX_HARDWARE_RESET, i, thisHourhardwareReset.length]);

        var thisHourOffline = data_wiFiConnected.filter(x => {
            time = x.timestamp;
            time.startOf('hour');
            return time.isSame(axisTime);
        });
        data.push([INDEX_OFFLINE, i, thisHourOffline.length > 0 ? 0 : 1]);
    }

    //S3 stats processing
    s3FilesGrouppedByHour = [];
    xAxis.forEach(timestamp => {
        var s3FilesForHour = graphData_data_s3_Files.filter(f => f.t.isSame(timestamp));
        s3FilesGrouppedByHour.push(s3FilesForHour);
    });
    for (var i = 0; i < s3FilesGrouppedByHour.length; i++) {
        var currentS3FilesInHour = s3FilesGrouppedByHour[i];
        data.push([INDEX_S3_FILE_COUNT,
            i,
            currentS3FilesInHour.length
        ]);
        data.push([INDEX_S3_FILE_SIZE,
            i,
            currentS3FilesInHour.reduce(function (acc, currentVal) {
                return acc + currentVal.size;
            }, 0)
        ]);
    }


    //xAxis is category, so change to string
    xAxis = xAxis.map(date => date.format('MMM DD HH:mm'));
    var option = {
        tooltip: {
            trigger: 'axis',
            formatter: function (params) {
                params = params[0];
                if (params.seriesIndex == INDEX_S3_FILE_SIZE) {
                    return (params.value[1]/1024).toFixed(0) + ' kiB @ ' + params.name;
                }
                return params.value[1] + ' @ ' + params.name;
            },
            axisPointer: {
                animation: false
            }
        },
        title: [],
        singleAxis: [],
        series: []
    };

    echarts.util.each(indicators, function (xAx, idx) {
        option.title.push({
            textBaseline: 'middle',
            top: (idx + 0.5) * 100 / indicators.length + '%',
            text: xAx
        });
        option.singleAxis.push({
            left: 150,
            type: 'category',
            boundaryGap: false,
            data: xAxis,
            top: (idx * 100 / indicators.length + 5) + '%',
            height: (100 / indicators.length - 10) + '%',
        });
        option.series.push({
            singleAxisIndex: idx,
            coordinateSystem: 'singleAxis',
            type: 'scatter',
            data: [],
            symbolSize: function (dataItem) {
                if (idx == INDEX_S3_FILE_COUNT || idx == INDEX_MQTT_OK || idx == INDEX_MQTT_FAIL) {
                    return dataItem[1] * 2;
                }
                if (idx == INDEX_S3_FILE_SIZE) {
                    return Math.sqrt(dataItem[1]/1024);
                }
                return dataItem[1] * 6;
            }
        });
    });

    echarts.util.each(data, function (dataItem) {
        option.series[dataItem[0]].data.push([dataItem[1], dataItem[2]]);
    });

    connectivityGraph.setOption(option);
}

function numToHumanStr(number, suffix) {
    if (number == 0) {
        return "";
    } else {
        if (number == 1) {
            return number + " " + suffix;
        } else {
            return number + " " + suffix + "s";
        }
    }
}

function handleS3Data(s3Data) {
    var preparedData = DataTransformer.PrepareData(s3Data);
    if (!preparedData.grouppedData) {
        alert("No data found for given range!");
        GraphsManager.MarkLoaded();
        return;
    }
    TilesRefresher.RefreshTopTiles(preparedData.maxDate, preparedData.grouppedData.CatWeight);

    drawCatWeight(preparedData.grouppedData.CatWeight);
    drawEatingStats(preparedData.grouppedData.EatingStats, preparedData.minDate, preparedData.maxDate);
    drawBeforeLidOpeningStats(preparedData.grouppedData.BeforeLidOpeningStats);
    drawCatToBowlTime(preparedData.grouppedData.CatToBowlTime);
    drawStepsPerLidOperation(preparedData.grouppedData.StepsPerLidOperation);
    drawStepsPerFoodGram(preparedData.grouppedData.StepsPerFoodGram);
    //not used (showes progress how food is eaten):
    //var data_foodAmount = s3Data.filter(s3DataObject => s3DataObject.type === "FoodAmount");

    //every minute stats
    drawTemperatureGraph(preparedData.grouppedData.Temp);
    drawFreeHeapSizeGraph(preparedData.grouppedData.MinFreeHeapSize);
    drawMaxProtothreadLoopLengthGraphs(preparedData.grouppedData.ProtothreadLoopLength);
    drawMessageBufferSizeGraph(preparedData.grouppedData.MessageBufferUtilization);

    //every day stats
    drawClockDriftGraph(preparedData.grouppedData.ClockDrift);

    drawConnectivityGraph(
        preparedData.grouppedData.MqttPublishedSuccessfully,
        preparedData.grouppedData.MqttPublishError,
        preparedData.grouppedData.HardwareReset,
        preparedData.grouppedData.WiFiConnected,
        preparedData.minDate, preparedData.maxDate);

    drawReverseFoodMotorSteps(preparedData.grouppedData.ReverseFoodMotorSteps);
    drawAfterEatingFoodAmount(preparedData.grouppedData.AfterEatingFoodAmount);

    GraphsManager.MarkLoaded();
}

function init_daterangepicker() {
    if (typeof ($.fn.daterangepicker) === 'undefined') { return; }
    console.log('init_daterangepicker');

    var cb = function (start, end, label) {
        console.log("daterangepicker callback: start: " + start.toISOString() + " end: " + end.toISOString() + " label: " + label);
        $('#reportrange span').html(start.format('MMMM D, YYYY') + ' - ' + end.format('MMMM D, YYYY'));
        GraphsManager.MarkLoading();
        DataFetcher.getDataFrom(start, end, storeWrittenS3FilesGraphData, handleS3Data);
    };

    var optionSet1 = {
        startDate: moment().subtract(2, 'days'),
        endDate: moment(),
        minDate: '01/01/2018',
        maxDate: '01/01/2025',
        dateLimit: {
            days: 370
        },
        showDropdowns: true,
        showWeekNumbers: true,
        timePicker: false,
        timePickerIncrement: 1,
        timePicker12Hour: true,
        ranges: {
            'Last 2 Days': [moment().subtract(2, 'days'), moment()],
            'Last 7 Days': [moment().subtract(6, 'days'), moment()],
            'Last 30 Days': [moment().subtract(30, 'days'), moment()],
            'Last 60 Days': [moment().subtract(60, 'days'), moment()],
            'Last 90 Days': [moment().subtract(90, 'days'), moment()],
            'Last Year': [moment().subtract(365, 'days'), moment()],
        },
        opens: 'left',
        buttonClasses: ['btn btn-default'],
        applyClass: 'btn-small btn-primary',
        cancelClass: 'btn-small',
        format: 'MM/DD/YYYY',
        separator: ' to ',
        locale: {
            applyLabel: 'Submit',
            cancelLabel: 'Clear',
            fromLabel: 'From',
            toLabel: 'To',
            customRangeLabel: 'Custom',
            daysOfWeek: ['Su', 'Mo', 'Tu', 'We', 'Th', 'Fr', 'Sa'],
            monthNames: ['January', 'February', 'March', 'April', 'May', 'June', 'July', 'August', 'September', 'October', 'November', 'December'],
            firstDay: 1
        }
    };

    $('#reportrange span').html(moment().subtract(29, 'days').format('MMMM D, YYYY') + ' - ' + moment().format('MMMM D, YYYY'));
    $('#reportrange').daterangepicker(optionSet1, cb);
    $('#reportrange').on('show.daterangepicker', function () {
        console.log("show event fired");
    });
    $('#reportrange').on('hide.daterangepicker', function () {
        console.log("hide event fired");
    });
    $('#reportrange').on('apply.daterangepicker', function (ev, picker) {
        console.log("apply event fired, start/end dates are " + picker.startDate.format('MMMM D, YYYY') + " to " + picker.endDate.format('MMMM D, YYYY'));
    });
    $('#reportrange').on('cancel.daterangepicker', function (ev, picker) {
        console.log("cancel event fired");
    });
    $('#destroy').click(function () {
        $('#reportrange').data('daterangepicker').remove();
    });

}

$(document).ready(function () {
    // Progressbar
    if ($(".progress .progress-bar")[0]) {
        $('.progress .progress-bar').progressbar();
    }
    // /Progressbar


    init_daterangepicker();
    GraphsManager.Init();
    GraphsManager.MarkLoading();
    DataFetcher.getDataFrom(moment().subtract(2, 'days'), moment(), storeWrittenS3FilesGraphData, handleS3Data);
});
(function graphsManager() {
    window.GraphsManager = {};

    window.catWeightLineEChartGraph = null;
    window.foodGivenLineEChartGraph = null;
    window.catToBowlTimeChartGraph = null;
    window.eatingSpeedChartGraph = null;
    window.stepsPerLidOperationGraph = null;
    window.stepsPerFoodGramGraph = null;
    window.temperatureGraph = null;
    window.freeHeapSizeGraph = null;
    window.maxProtothreadLoopLengthGraphLong = null;
    window.maxProtothreadLoopLengthGraphQuick = null;
    window.messageSizeGraph = null;
    window.clockDriftGraph = null;
    window.connectivityGraph = null;
    window.reverseFoodMotorSteps = null;
    window.afterEatingFoodAmount = null;

    window.GraphsManager.Init = function GraphsManagerInit() {
        console.log('init_echarts');

        catWeightLineEChartGraph = echarts.init(document.getElementById('catWeightLineEChartGraph'), theme);
        foodGivenLineEChartGraph = echarts.init(document.getElementById('foodGivenLineEChartGraph'), theme);
        catToBowlTimeChartGraph = echarts.init(document.getElementById('catToBowlTimeChartGraph'), theme);
        eatingSpeedChartGraph = echarts.init(document.getElementById('eatingSpeedChartGraph'), theme);
        stepsPerLidOperationGraph = echarts.init(document.getElementById('stepsPerLidOperationGraph'), theme);
        stepsPerFoodGramGraph = echarts.init(document.getElementById('stepsPerFoodGramGraph'), theme);
        temperatureGraph = echarts.init(document.getElementById('temperatureGraph'), theme);
        freeHeapSizeGraph = echarts.init(document.getElementById('freeHeapSizeGraph'), theme);
        maxProtothreadLoopLengthGraphLong = echarts.init(document.getElementById('maxProtothreadLoopLengthGraphLong'), theme);
        maxProtothreadLoopLengthGraphQuick = echarts.init(document.getElementById('maxProtothreadLoopLengthGraphQuick'), theme);
        messageSizeGraph = echarts.init(document.getElementById('messageSizeGraph'), theme);
        clockDriftGraph = echarts.init(document.getElementById('clockDriftGraph'), theme);
        connectivityGraph = echarts.init(document.getElementById('connectivityGraph'), theme);
        reverseFoodMotorSteps = echarts.init(document.getElementById('reverseFoodMotorSteps'), theme);
        afterEatingFoodAmount = echarts.init(document.getElementById('afterEatingFoodAmount'), theme);
    }

    window.GraphsManager.MarkLoading = function GraphsManagerMarkLoading() {
        catWeightLineEChartGraph.showLoading();
        foodGivenLineEChartGraph.showLoading();
        catToBowlTimeChartGraph.showLoading();
        eatingSpeedChartGraph.showLoading();
        stepsPerLidOperationGraph.showLoading();
        stepsPerFoodGramGraph.showLoading();
        temperatureGraph.showLoading();
        freeHeapSizeGraph.showLoading();
        maxProtothreadLoopLengthGraphLong.showLoading();
        maxProtothreadLoopLengthGraphQuick.showLoading();
        messageSizeGraph.showLoading();
        clockDriftGraph.showLoading();
        connectivityGraph.showLoading();
        reverseFoodMotorSteps.showLoading();
        afterEatingFoodAmount.showLoading();
    }
    
    window.GraphsManager.MarkLoaded = function GraphsManagerMarkLoaded() {
        catWeightLineEChartGraph.hideLoading();
        foodGivenLineEChartGraph.hideLoading();
        catToBowlTimeChartGraph.hideLoading();
        eatingSpeedChartGraph.hideLoading();
        stepsPerLidOperationGraph.hideLoading();
        stepsPerFoodGramGraph.hideLoading();
        temperatureGraph.hideLoading();
        freeHeapSizeGraph.hideLoading();
        maxProtothreadLoopLengthGraphLong.hideLoading();
        maxProtothreadLoopLengthGraphQuick.hideLoading();
        messageSizeGraph.hideLoading();
        clockDriftGraph.hideLoading();
        connectivityGraph.hideLoading();
        reverseFoodMotorSteps.hideLoading();
        afterEatingFoodAmount.hideLoading();
    }

    var theme = {
        color: [
            '#26B99A', '#34495E', '#BDC3C7', '#3498DB',
            '#9B59B6', '#8abb6f', '#759c6a', '#bfd3b7'
        ],

        title: {
            itemGap: 8,
            textStyle: {
                fontWeight: 'normal',
                color: '#408829'
            }
        },

        dataRange: {
            color: ['#1f610a', '#97b58d']
        },

        toolbox: {
            color: ['#408829', '#408829', '#408829', '#408829']
        },

        tooltip: {
            backgroundColor: 'rgba(0,0,0,0.5)',
            axisPointer: {
                type: 'line',
                lineStyle: {
                    color: '#408829',
                    type: 'dashed'
                },
                crossStyle: {
                    color: '#408829'
                },
                shadowStyle: {
                    color: 'rgba(200,200,200,0.3)'
                }
            }
        },

        dataZoom: {
            dataBackgroundColor: '#eee',
            fillerColor: 'rgba(64,136,41,0.2)',
            handleColor: '#408829'
        },
        grid: {
            borderWidth: 0
        },

        categoryAxis: {
            axisLine: {
                lineStyle: {
                    color: '#408829'
                }
            },
            splitLine: {
                lineStyle: {
                    color: ['#eee']
                }
            }
        },

        valueAxis: {
            axisLine: {
                lineStyle: {
                    color: '#408829'
                }
            },
            splitArea: {
                show: true,
                areaStyle: {
                    color: ['rgba(250,250,250,0.1)', 'rgba(200,200,200,0.1)']
                }
            },
            splitLine: {
                lineStyle: {
                    color: ['#eee']
                }
            }
        },
        timeline: {
            lineStyle: {
                color: '#408829'
            },
            controlStyle: {
                normal: { color: '#408829' },
                emphasis: { color: '#408829' }
            }
        },

        k: {
            itemStyle: {
                normal: {
                    color: '#68a54a',
                    color0: '#a9cba2',
                    lineStyle: {
                        width: 1,
                        color: '#408829',
                        color0: '#86b379'
                    }
                }
            }
        },
        map: {
            itemStyle: {
                normal: {
                    areaStyle: {
                        color: '#ddd'
                    },
                    label: {
                        textStyle: {
                            color: '#c12e34'
                        }
                    }
                },
                emphasis: {
                    areaStyle: {
                        color: '#99d2dd'
                    },
                    label: {
                        textStyle: {
                            color: '#c12e34'
                        }
                    }
                }
            }
        },
        force: {
            itemStyle: {
                normal: {
                    linkStyle: {
                        strokeColor: '#408829'
                    }
                }
            }
        },
        chord: {
            padding: 4,
            itemStyle: {
                normal: {
                    lineStyle: {
                        width: 1,
                        color: 'rgba(128, 128, 128, 0.5)'
                    },
                    chordStyle: {
                        lineStyle: {
                            width: 1,
                            color: 'rgba(128, 128, 128, 0.5)'
                        }
                    }
                },
                emphasis: {
                    lineStyle: {
                        width: 1,
                        color: 'rgba(128, 128, 128, 0.5)'
                    },
                    chordStyle: {
                        lineStyle: {
                            width: 1,
                            color: 'rgba(128, 128, 128, 0.5)'
                        }
                    }
                }
            }
        },
        gauge: {
            startAngle: 225,
            endAngle: -45,
            axisLine: {
                show: true,
                lineStyle: {
                    color: [[0.2, '#86b379'], [0.8, '#68a54a'], [1, '#408829']],
                    width: 8
                }
            },
            axisTick: {
                splitNumber: 10,
                length: 12,
                lineStyle: {
                    color: 'auto'
                }
            },
            axisLabel: {
                textStyle: {
                    color: 'auto'
                }
            },
            splitLine: {
                length: 18,
                lineStyle: {
                    color: 'auto'
                }
            },
            pointer: {
                length: '90%',
                color: 'auto'
            },
            title: {
                textStyle: {
                    color: '#333'
                }
            },
            detail: {
                textStyle: {
                    color: 'auto'
                }
            }
        },
        textStyle: {
            fontFamily: 'Arial, Verdana, sans-serif'
        }
    };
})();
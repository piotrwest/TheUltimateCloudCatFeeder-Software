(function dataFetcher() {
    AWS.config.update({
        region: 'us-east-1'
    });
    var s3 = new AWS.S3({
        params: {
            Bucket: CatFeederConfig.s3Bucket
        }
    });
    
    window.DataFetcher = {};
    window.DataFetcher.getDataFrom = function getDataFrom(start, end, s3KeysFetchedCallback, dataFetchedCallback) {
        var startDay = start.format('YYYY-MM-DD');
        var startEndTime = moment().format('THH:mm:ss.SSS');
        var endDay = end.format('YYYY-MM-DD');
        start = moment(startDay + startEndTime);
        end = moment(endDay + startEndTime);
        console.log("Getting data from S3 from (local):", start.format(), end.format());
        console.log("Getting data from S3 from (UTC):", start.utc().toISOString(), end.utc().toISOString());

        if (end.isSameOrBefore(start)) {
            var tmp = start;
            start = end;
            end = tmp;
        }

        start = start.utc(); //make sure it's utc - this is how S3 files are indexed
        end = end.utc();
        var startMonth = moment(start).startOf('month');
        var endMonth = moment(end).startOf('month');
        var allS3Prefixes = [];
        while (startMonth.isSameOrBefore(endMonth)) {
            allS3Prefixes.push('data/' + startMonth.format("YYYY/MM/"));
            startMonth = startMonth.add(1, "month");
        };
        console.log('Will list prefixes: ' + allS3Prefixes);

        var listS3FilesPromises = [];
        allS3Prefixes.forEach(s3Prefix => {
            var params = {
                MaxKeys: 100000,
                Prefix: s3Prefix
            };
            listS3FilesPromises.push(
                new Promise((resolve, reject) => {
                    s3.makeUnauthenticatedRequest('listObjectsV2', params, function(err, data) {
                        if (err) {
                            reject(err);
                        } else {
                            resolve(data);
                        }
                    });
                })
            );
        });

        var s3Keys = [];
        function handlePromissesResults(values) {
            var newPromises = [];
            values.forEach(v => {
                if (v.IsTruncated) {
                    var params = {
                        MaxKeys: v.MaxKeys,
                        Prefix: v.Prefix,
                        ContinuationToken: v.NextContinuationToken
                    };
                    console.log('Got truncated result. Current keys length: ' + s3Keys.length + ' Promises to continue: ' + newPromises.length);
                    newPromises.push(
                        new Promise((resolve, reject) => {
                            s3.makeUnauthenticatedRequest('listObjectsV2', params, function(err, data) {
                                if (err) {
                                    reject(err);
                                } else {
                                    resolve(data);
                                }
                            });
                        })
                    );
                }
            });
            s3Keys = s3Keys.concat(values.flatMap(v => v.Contents));
            if (newPromises.length > 0) {
                console.log('Continuing # of promises: ' + newPromises.length);
                Promise.all(newPromises).then(handlePromissesResults);
            } else {
                console.log('# of s3 fetched: ' + s3Keys.length);
                s3KeysFetchedCallback(s3Keys);
                var getS3FilesPromises = [];
                s3Keys.forEach(key => {
                    //filter each file if belongs to <start, end>
                    var urlKey = key.Key;
                    var isoDateLength = "2019-02-08T00:46:04.600Z".length;
                    var keyTimestamp = urlKey.substr(urlKey.length - isoDateLength - ".json".length, isoDateLength);
                    var timestamp = moment(keyTimestamp);
                    if (timestamp.isSameOrAfter(start) && timestamp.isSameOrBefore(end)) {
                        console.log('Key: ' + urlKey + '\t IS BETWEEN <' + start.toISOString() + ', ' + end.toISOString() + '>');
                        getS3FilesPromises.push(Promise.resolve($.get(CatFeederConfig.s3Url + urlKey)));
                    } else {
                        console.log('Key: ' + urlKey + '\t IS NOT BETWEEN <' + start.toISOString() + ', ' + end.toISOString() + '>');
                    }
                });
                Promise.all(getS3FilesPromises).then(dataFetchedCallback);
            }
        };

        Promise.all(listS3FilesPromises).then(handlePromissesResults);
    }
})();
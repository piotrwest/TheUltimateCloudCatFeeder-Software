(function semiPrivateConfig() {
    window.CatFeederConfig = {};
    window.CatFeederConfig.s3Bucket = 'REPLACE_WITH_YOUR_BUCKET_NAME';
    window.CatFeederConfig.s3Url = 'http://REPLACE_WITH_YOUR_BUCKET_NAME.s3-website-us-east-1.amazonaws.com/';
    window.CatFeederConfig.CatMinWeight = 3300; //replace with minimum "feasible" cat weight
    window.CatFeederConfig.CatMaxWeight = 4800; //replace with maximum "feasible" cat weight
    window.CatFeederConfig.CatBirthday = moment("2011-06-10");
})();
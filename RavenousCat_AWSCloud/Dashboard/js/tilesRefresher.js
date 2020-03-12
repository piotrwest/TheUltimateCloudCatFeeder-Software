(function tilesRefresher() {
    var totalMaxDate = null;
    var lastCatWeightItem = null;

    window.TilesRefresher = {};
    window.TilesRefresher.RefreshTopTiles = function RefreshTopTiles(maxDate, data_catWeight) {
        if (!data_catWeight || data_catWeight.length == 0) {
            return;
        }
        var maxFromCatWeightThisTime = data_catWeight.reduce(function(prev, curr) {
            return prev.timestamp.isSameOrAfter(curr.timestamp) ? prev : curr;
        });

        if (!totalMaxDate) {
            totalMaxDate = maxDate;
            lastCatWeightItem = maxFromCatWeightThisTime;
        } else {
            if (maxDate.isSameOrAfter(totalMaxDate)) {
                totalMaxDate = maxDate;
            }
            if (maxFromCatWeightThisTime.timestamp.isSameOrAfter(lastCatWeightItem.timestamp)) {
                lastCatWeightItem = maxFromCatWeightThisTime;
            }
        }

        $('#lastUpdateTimeTile').text(totalMaxDate.fromNow());
        $('#lastCatWeightTile').text(lastCatWeightItem.value + "g");

        var currentDate = moment();
        currentDate.startOf('day');

        var diffFromBirthdays = moment.duration(currentDate.diff(CatFeederConfig.CatBirthday));
        $('#ageTile').text(numToHumanStr(diffFromBirthdays.years(), 'year') + " " + numToHumanStr(diffFromBirthdays.months(), "month"));

        var birthdaysThisYear = moment([currentDate.year(), CatFeederConfig.CatBirthday.month(), CatFeederConfig.CatBirthday.date()]);
        if (birthdaysThisYear.isSame(currentDate)) {
            $('#birthdaysTile').text("Let's celebrate!");
        } else {
            if (birthdaysThisYear.isBefore(currentDate)) {
                birthdaysThisYear.add(1, "years");
            }
            var diff = moment.duration(birthdaysThisYear.diff(currentDate));
            $('#birthdaysTile').text(numToHumanStr(diff.months(), "month") + " " + numToHumanStr(diff.days(), "day"));
        }
    }
})();
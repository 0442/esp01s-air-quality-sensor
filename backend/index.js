const express = require('express')

const app = express()
const port = 80
const root_path = __dirname
const sqlite = require('sqlite3')

const db = new sqlite.Database("logs.db", [sqlite.OPEN_READONLY])
/**
 * get pm value logs from db
 * @param {string} timeFrame time frame for which readings are to be returned. Valid values are "minute", "hour", "day", "week" and "month".
 * @param {} callback callback function.
 */
const pm_readings = (timeFrame, callback) => {
    sql_get_data = `
        SELECT
            value, timestamp
        FROM logs
            WHERE strftime('%s') - timestamp <= ?;
    `
    sql_get_data1 = `
        SELECT value, unixepoch(timestamp) as timestamp FROM logs;
    `
    var oldest = 0
    if (timeFrame === "minute") {
        oldest = 60
    } else if (timeFrame === "hour") {
        oldest = 60 * 60
    } else if (timeFrame === "day") {
        oldest = 60 * 60 * 24
    } else if (timeFrame === "week") {
        oldest = 60 * 60 * 24 * 7
    } else if (timeFrame === "month") {
        oldest = 60 * 60 * 24 * 30
    }

    if (oldest === 0) {
        throw Error("Error while getting readings from db: invalid time frame.")
    }

    db.all(sql_get_data, [oldest], (err, rows) => {
        console.log("db query done")
        console.log(rows)
        callback(err, rows)
    })

}

/**
 * Calculate average pm readings for given time interval
 * @param {int} interval interval in seconds. interval for which to get all readings within and to calculate the average reading.
 * @param {int} maxAge in seconds, how far behind to include readings.
 */
const avg_pm_readings = (interval, maxAge, callback) => {
    const sql_get_data = `
        SELECT value, timestamp
        FROM logs
        WHERE strftime('%s') - timestamp <= ?
    `

    db.all(sql_get_data, [maxAge], (err, rows) => {
        if (rows.length < 1) { return }
        var referenceTime = rows[0].timestamp
        var avgs = []
        var curSum = 0
        var curSumCount = 0

        for (let i = 0; i < rows.length; i++) {
            const value = rows[i].value
            const timestamp = rows[i].timestamp
            if (timestamp - referenceTime <= interval) {
                curSum += value
                curSumCount += 1
                continue
            }

            const avg = curSum / curSumCount
            avgs.push({ value: avg, timestamp: referenceTime + interval })

            referenceTime += interval
            curSum = 0
            curSumCount = 0
        }

        console.log("avgs:", avgs)
        callback(err, avgs)
    })
}

app.listen(port, () => {
    console.log("Server running on port " + port)
})

// log all reqs
app.use((req, res, next) => {
    console.log(`<< ${req.method} to ${req.path} from ${req.ip} `)
    next()
})

// files
app.get("/", (req, res) => res.status(200).sendFile(root_path + "/index.html"))
app.get("/chart.umd.js", (req, res) => res.status(200).sendFile(root_path + "node_modules/chart.js/dist/chart.umd.js"))
app.get("/who-aqg.jpg", (req, res) => res.status(200).sendFile(root_path + "/who-aqg.jpg"))
app.get("/particle-examples.svg", (req, res) => res.status(200).sendFile(root_path + "/Airborne-particulate-size-chart.svg"))
app.get("/pm1006.png", (req, res) => res.status(200).sendFile(root_path + "/pm1006.png"))

// data
app.get("/data/minute", (req, res) => {
    pm_readings("minute", (err, r) => {
        res.status(200).send({ data: r })
    })
})
app.get("/data/hour", (req, res) => {
    avg_pm_readings(60, 60 * 60, (err, r) => {
        res.status(200).send({ data: r })
    })
})
app.get("/data/day", (req, res) => {
    avg_pm_readings(60 * 60, 60 * 60 * 24, (err, r) => {
        res.status(200).send({ data: r })
    })
})
app.get("/data/week", (req, res) => {
    avg_pm_readings(60 * 60 * 24, 60 * 60 * 24 * 7, (err, r) => {
        res.status(200).send({ data: r })
    })
})
app.get("/data/month", (req, res) => {
    avg_pm_readings(60 * 60 * 24, 60 * 60 * 24 * 30, (err, r) => {
        res.status(200).send({ data: r })
    })
})

app.use((err, req, res, next) => {
    res.status(500).send('error')
    console.log(err)
    console.log(`>> response ${res.method} ${res.statusCode} error`)
})

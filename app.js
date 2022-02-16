'use strict'

// C library API
const ffi = require('ffi-napi');

// Express App (Routes)
const express = require("express");
const app = express();
const path = require("path");
const fileUpload = require('express-fileupload');

app.use(fileUpload());
app.use(express.static(path.join(__dirname + '/uploads')));
app.use(express.urlencoded({ extended: true }))
// Minimization
const fs = require('fs');
const JavaScriptObfuscator = require('javascript-obfuscator');
const e = require('express');
const { parse } = require('path');

// note... dotenv would be more secure... however, this is the cis2750 hackathon soo...
// creates the db connection
const mysql = require('mysql2/promise');
const { query } = require('express');

// Important, pass in port as in `npm run dev 1234`, do not change
const portNum = process.argv[2];

// Send HTML at root, do not change
app.get('/', function (req, res) {
  res.sendFile(path.join(__dirname + '/public/index.html'));
});

// Send Style, do not change
app.get('/style.css', function (req, res) {
  //Feel free to change the contents of style.css to prettify your Web app
  res.sendFile(path.join(__dirname + '/public/style.css'));
});

// Send obfuscated JS, do not change
app.get('/index.js', function (req, res) {
  fs.readFile(path.join(__dirname + '/public/index.js'), 'utf8', function (err, contents) {
    const minimizedContents = JavaScriptObfuscator.obfuscate(contents, { compact: true, controlFlowFlattening: true });
    res.contentType('application/javascript');
    res.send(minimizedContents._obfuscatedCode);
  });
});

//Respond to POST requests that upload files to uploads/ directory
app.post('/upload', function (req, res) {
  if (!req.files) {
    return res.status(400).send('No files were uploaded.');
  }

  let uploadFile = req.files.uploadFile;
  // Use the mv() method to place the file somewhere on your server
  uploadFile.mv('uploads/' + uploadFile.name, function (err) {
    if (err) {
      return res.status(500).send(err);
    }
    res.redirect('/');
  });
});

//Respond to GET requests for files in the uploads/ directory
app.get('/uploads/:name', function (req, res) {
  fs.stat('uploads/' + req.params.name, function (err, stat) {
    if (err == null) {
      res.sendFile(path.join(__dirname + '/uploads/' + req.params.name));
    } else {
      console.log('Error in file downloading route: ' + err);
      res.send('');
    }
  });
});

//******************** Your code goes here ******************** 

var parserfuncs = ffi.Library('./libgpxparser.so', {
  'GPXFiletoJSON': ['string', ['string', 'string']],
  'GPXFiletoJSONDetailed': ['string', ['string', 'string']],
  'createDotGPX': ['bool', ['string', 'string', 'string']],
  'updateRouteName': ['bool', ['string', 'string', 'string', 'string']],
  'updateTrackName': ['bool', ['string', 'string', 'string', 'string']],
  'getRteGpxData': ['string', ['string', 'string', 'string']],
  'getTrkGpxData': ['string', ['string', 'string', 'string']],
  'routesBetweenToJSON': ['string', ['string', 'string', 'float', 'float', 'float', 'float', 'float']],
  'tracksBetweenToJSON': ['string', ['string', 'string', 'float', 'float', 'float', 'float', 'float']],
  'validateLibXmlTree': ['bool', ['string', 'string']],
  'routesofLenJSON': ['int', ['string', 'string', 'float']],
  'tracksofLenJSON': ['int', ['string', 'string', 'float']],
  'addNewRoute': ['bool', ['string', 'string', 'string', 'string']],
})

//Sample endpoint
app.get('/endpoint1', function (req, res) {
  let retStr = req.query.data1 + " " + req.query.data2;
  res.send(
    {
      somethingElse: retStr
    }
  );
});

app.delete('/clearDB', async (req, res) => {
  let userName = req.body.username;
  let password = req.body.password;
  let hostname = req.body.hostname;
  let database = req.body.database;

  let clear_point_str = `DELETE FROM POINT WHERE point_id > 0`;
  let clear_rte_str = `DELETE FROM ROUTE WHERE route_id > 0`;
  let clear_file_str = `DELETE FROM FILE WHERE gpx_id > 0`;

  let connection;
  try {
    connection = await mysql.createConnection({
      host: hostname,
      user: userName,
      password: password,
      database: database,
    });
    await connection.execute(clear_file_str);
    await connection.execute(clear_rte_str);
    await connection.execute(clear_point_str);
    res.send({ message: "Successfully Cleared tables \n" })
  } catch (e) {
    console.log("Query error: " + e);
    res.status(500).send({ message: "Unsuccessful Connection to MYSQL Database \n" + e })
  } finally {
    if (connection && connection.end) connection.end();
  }
})

app.put('/uploadDB', async (req, res) => {
  let userName = req.body.username;
  let password = req.body.password;
  let hostname = req.body.hostname;
  let database = req.body.database;
  let connection;
  let parsedFiles = [];
  let names = [];
  try { // try to connect to mysql
    connection = await mysql.createConnection({
      host: hostname,
      user: userName,
      password: password,
      database: database,
    });

    let filenames = fs.readdirSync(__dirname + '/uploads')

    filenames.map(file => {
      if (file.includes('.gpx') && parserfuncs.validateLibXmlTree(__dirname + '/uploads/' + file, './gpx.xsd')) {
        let parsedFile = JSON.parse(parserfuncs.GPXFiletoJSONDetailed(__dirname + '/uploads/' + file, './gpx.xsd'))
        names.push(file)
        parsedFiles.push(parsedFile)
      }
    })
    for (var i = 0; i < parsedFiles.length; i++) { // look through all parsed files... push to database
      let tmpfile = parsedFiles[i];

      let file_insert_string = `INSERT INTO FILE (file_name, ver, creator) VALUES ("${names[i]}", ${tmpfile.version}, "${tmpfile.creator}")`;
      await connection.execute(file_insert_string)

      let gpx_id = await connection.execute('SELECT LAST_INSERT_ID() gpxid')
      gpx_id = gpx_id[0][0].gpxid
      console.log("gpx_id")
      console.log(gpx_id);
      for (var j = 0; j < tmpfile.routes.length; j++) { // loop through all routes and push to db
        let route = tmpfile.routes[j];
        let route_insert_string = `INSERT INTO ROUTE (route_name, route_len, gpx_id) VALUES ("${route.name}", ${route.len}, ${gpx_id})`
        await connection.execute(route_insert_string);
        let route_id = await connection.execute('SELECT LAST_INSERT_ID() routeid')
        route_id = route_id[0][0].routeid
        console.log("Route id:")
        console.log(route_id)
        for (var k = 0; k < route.waypoints.length; k++) { // loop through all points and push to db
          let point = route.waypoints[k];
          let point_insert_string = `INSERT INTO POINT (point_index, latitude, longitude, point_name, route_id) VALUES (${k + 1}, ${point.lat}, ${point.lon}, "${point.name}", ${route_id})`
          connection.execute(point_insert_string)
        }
      }
    }
    res.send({ message: "Successfully Uploaded Files \n" })
  } catch (e) {
    console.log("Query error: " + e);
    res.status(500).send({ message: "Unsuccessful Connection to MYSQL Database \n" + e })
  } finally {
    if (connection && connection.end) connection.end();
  }
})

app.get('/dbStats', async (req, res) => {
  //console.log(req.query)
  let userName = req.query.username;
  let password = req.query.password;
  let hostname = req.query.hostname;
  let database = req.query.database;

  let count_route_rows = "SELECT COUNT(*) rows FROM ROUTE";
  let count_point_rows = "SELECT COUNT(*) rows FROM POINT";
  let count_file_rows = "SELECT COUNT(*) rows FROM FILE";

  let num_routes_rows = 0;
  let num_points_rows = 0;
  let num_files_rows = 0;

  let connection;
  try {
    connection = await mysql.createConnection({
      host: hostname,
      user: userName,
      password: password,
      database: database,
    });
    const [routeRows, fields1] = await connection.execute(count_route_rows);
    const [pointsRows, fields2] = await connection.execute(count_point_rows);
    const [fileRows, fields3] = await connection.execute(count_file_rows);
    //console.log(routeRows[0].rows)
    //console.log(pointsRows[0].rows)
    res.send({
      message: "Database Stats \n",
      routeRows: routeRows[0].rows,
      pointRows: pointsRows[0].rows,
      fileRows: fileRows[0].rows,
    })
  } catch (e) {
    console.log("Query error: " + e);
    res.status(500).send({ message: "Unsuccessful Connection to MYSQL Database \n" + e })
  } finally {
    if (connection && connection.end) connection.end();
  }
})

// used for user login to database
app.post('/login', async (req, res) => {
  let userName = req.headers.username;
  let password = req.headers.password;
  let hostname = req.headers.hostname;
  let database = req.headers.database;
  let file_table_str = `CREATE TABLE IF NOT EXISTS FILE` +
    "(gpx_id INT AUTO_INCREMENT PRIMARY KEY," +
    "file_name VARCHAR(60)," +
    "ver decimal(2, 1) NOT NULL," +
    "creator varchar(256) NOT NULL)";
  let route_table_str = "CREATE TABLE IF NOT EXISTS ROUTE" +
    "(route_id INT AUTO_INCREMENT," +
    "route_name VARCHAR(256)," +
    "route_len FLOAT(15, 7) NOT NULL," +
    "gpx_id INT NOT NULL," +
    "PRIMARY KEY(route_id)," +
    "FOREIGN KEY(gpx_id) REFERENCES FILE(gpx_id) ON DELETE CASCADE)"
  let point_table_str = "CREATE TABLE IF NOT EXISTS POINT" +
    "(point_id INT AUTO_INCREMENT PRIMARY KEY," +
    "point_index INT NOT NULL," +
    "latitude DECIMAL(11, 7) NOT NULL," +
    "longitude DECIMAL(11, 7) NOT NULL," +
    "point_name VARCHAR(256)," +
    "route_id INT NOT NULL," +
    "FOREIGN KEY(route_id) REFERENCES ROUTE(route_id) ON DELETE CASCADE)"
  let connection;
  try {
    connection = await mysql.createConnection({
      host: hostname,
      user: userName,
      password: password,
      database: database,
    });
    await connection.execute(file_table_str);
    await connection.execute(route_table_str);
    await connection.execute(point_table_str)
    res.send({ message: "Successfully connected to MYSQL Database \n" })
  } catch (e) {
    console.log("Query error: " + e);
    res.status(500).send({ message: "Unsuccessful Connection to MYSQL Database \n" + e })
  } finally {
    if (connection && connection.end) connection.end();
  }
});

// this function handles quering the MYSQL database
app.get('/queryDB', async (req, res) => {
  //console.log(req.query)
  let fileName = req.query.fileName;
  let routeCount = parseInt(req.query.routeCount)
  let routeName = req.query.routeName;
  let queryType = req.query.queryType;

  let queryStr;
  let route_id;
  let file_id;
  let id;

  let error = false;

  let connection;
  if (!fileName.includes('.gpx')) { // concats .gpx when the file does not contain one
    fileName = fileName + '.gpx'
  }
  try {
    connection = await mysql.createConnection({
      host: req.query.hostname,
      user: req.query.username,
      password: req.query.password,
      database: req.query.database,
    });
    //console.log(queryType)
    switch (queryType) {
      case "AllRte":
        queryStr = "SELECT * FROM ROUTE"
        break;
      case "SpecificRte":
        file_id = `SELECT gpx_id FROM FILE WHERE file_name = "${fileName}"`
        id = await connection.execute(file_id);
        id = id[0][0].gpx_id
        queryStr = `SELECT * FROM ROUTE WHERE gpx_id = ${id}`
        //console.log(id[0][0].gpx_id);
        break;
      case "NShortRte":
        if (routeCount < 0) {
          error = true;
          res.status(500).send({ message: "Invalid Number of Routes \n" })
        }
        queryStr = `SELECT * FROM ROUTE ORDER BY route_len ASC LIMIT ${routeCount}`
        break;
      case "NLongRte":
        if (routeCount < 0) {
          error = true;
          res.status(500).send({ message: "Invalid Number of Routes \n" })
        }
        queryStr = `SELECT * FROM ROUTE ORDER BY route_len DESC LIMIT ${routeCount}`
        break;
      case "SpecificPts":
        route_id = `SELECT route_id FROM ROUTE WHERE route_name = "${routeName}"`
        id = await connection.execute(route_id);
        queryStr = `SELECT * FROM POINT WHERE route_id = ${id[0][0].route_id} ORDER BY point_index`
        //console.log(id[0][0].route_id);
        break;
      case "FilePts":
        file_id = `SELECT gpx_id FROM FILE WHERE file_name = "${fileName}"`
        id = await connection.execute(file_id)
        route_id = await connection.execute(`SELECT route_id FROM ROUTE WHERE gpx_id = ${id[0][0].gpx_id}`)
        queryStr = `SELECT * FROM POINT WHERE route_id = ${route_id[0][0].route_id} ORDER BY point_index`
        break;
      default:
        break;
    }

    if (error === false) {
      const [rows, cols] = await connection.execute(queryStr)
      console.log(rows)
      res.send({
        message: "Successfully communicated with MYSQL Database",
        data: rows,
        queryType: queryType,
      })
    }

  } catch (e) {
    res.status(500).send({ message: "Unsuccessful Connection to MYSQL Database \n" + e })
  } finally {
    if (connection && connection.end) connection.end();
  }
})

app.get('/uploads/', function (req, res) {
  var parsed = [];
  var names = [];
  fs.readdir(__dirname + '/uploads', (err, files) => {
    if (err) {
      //console.log(err);
      res.status(400).send(err);
    } else {
      files.map(file => {
        if (file.includes('.gpx') && parserfuncs.validateLibXmlTree(__dirname + '/uploads/' + file, './gpx.xsd')) {
          //console.log(file)
          var tmp = parserfuncs.GPXFiletoJSON(__dirname + '/uploads/' + file, './gpx.xsd');
          if (tmp !== "{}") {
            parsed.push(JSON.parse(tmp));
            names.push(file);
          }
        }
        //console.log(tmp);
      })
      //console.log(parsed);
      names.push('');
      //console.log(names)
      res.send({
        gpx: parsed,
        names: names,
      })
    }
  })
});
// this route takes a specific file, returns a more detailed gpx obj
app.get('/detailedParse', async (req, res) => {
  var parsed = [];
  var tmp;
  tmp = parserfuncs.GPXFiletoJSONDetailed(__dirname + '/uploads/' + req.query.fn, './gpx.xsd')
  parsed.push(JSON.parse(tmp));
  //console.log(parsed[0].routes[0].waypoints)
  if (parsed !== []) {
    res.send({
      parsedGPX: parsed,
    });
  }
})

app.get('/getGpxData', async (req, res) => {
  var fn = req.query.fn;
  var type = req.query.type;
  var component = req.query.name;
  var ret;
  if (type === "route") {
    ret = parserfuncs.getRteGpxData(component, __dirname + '/uploads/' + fn, './gpx.xsd');
  }
  if (type === "track") {
    ret = parserfuncs.getTrkGpxData(component, __dirname + '/uploads/' + fn, './gpx.xsd');
  }
  if (ret) {
    res.send(
      {
        gpxData: JSON.parse(ret),
      }
    )
  } else {
    res.status.send({
      gpxData: 'No data'
    })
  }
})

app.post('/createRoute', async (req, res) => {
  var wpts = req.headers.waypoints
  var rte = `{"name":"${req.headers.rtename}"}`
  var fn = req.headers.fn
  var ret = parserfuncs.addNewRoute(wpts, rte, __dirname + '/uploads/' + fn, './gpx.xsd')
  if (ret) {
    res.send({ message: "Successfully created route" })
  } else {
    res.status(500).send({ error: "Server error" })
  }
})

app.get('/tracksBetween', async (req, res) => {
  var tracks = [];
  var names = [];
  var tmp = null;
  fs.readdir(__dirname + '/uploads/', (err, files) => {
    if (parseFloat(req.query.startLat) < -90.0 || parseFloat(req.query.startLat) > 90.0
      || parseFloat(req.query.startLon) < -180.0 || parseFloat(req.query.startLon) > 180.0) {
      res.status(500)
      console.log('Error')
    }
    if (parseFloat(req.query.destLat) < -90.0 || parseFloat(req.query.destLat) > 90.0
      || parseFloat(req.query.destLon) < -180.0 || parseFloat(req.query.destLon) > 180.0) {
      res.status(500)
      console.log('Error')
    }
    files.map(file => {
      if (file.includes('.gpx') && parserfuncs.validateLibXmlTree(__dirname + '/uploads/' + file, './gpx.xsd')) {
        tmp = parserfuncs.tracksBetweenToJSON(__dirname + '/uploads/' + file,
          './gpx.xsd',
          parseFloat(req.query.startLat),
          parseFloat(req.query.startLon),
          parseFloat(req.query.destLat),
          parseFloat(req.query.destLon),
          parseFloat(req.query.delta));
        if (tmp !== null) {
          tracks.push(JSON.parse(tmp))
          names.push(file);
        }
      }
    })
    res.send({
      names: names,
      tracks: tracks,
    })
  })
})

app.get('/routesBetween', async (req, res) => {
  var routes = [];
  var names = [];
  var tmp = null;
  //console.log(req.query)
  fs.readdir(__dirname + '/uploads/', (err, files) => {
    if (parseFloat(req.query.startLat) < -90.0 || parseFloat(req.query.startLat) > 90.0
      || parseFloat(req.query.startLon) < -180.0 || parseFloat(req.query.startLon) > 180.0) {
      res.status(500)
      console.log('Error')
    }
    if (parseFloat(req.query.destLat) < -90.0 || parseFloat(req.query.destLat) > 90.0
      || parseFloat(req.query.destLon) < -180.0 || parseFloat(req.query.destLon) > 180.0) {
      res.status(500)
      console.log('Error')
    }

    files.map(file => {
      if (file.includes('.gpx') && parserfuncs.validateLibXmlTree(__dirname + '/uploads/' + file, './gpx.xsd'))
        tmp = parserfuncs.routesBetweenToJSON(__dirname + '/uploads/' + file,
          './gpx.xsd',
          parseFloat(req.query.startLat),
          parseFloat(req.query.startLon),
          parseFloat(req.query.destLat),
          parseFloat(req.query.destLon),
          parseFloat(req.query.delta));
      if (tmp !== null) {
        routes.push(JSON.parse(tmp))
        names.push(file);
      }
    })
    //console.log(routes)
    //console.log(names)
    res.send({
      names: names,
      routes: routes,
    })
  })
})

app.get('/routesOfLen', async (req, res) => {
  var count = 0;
  if (parseFloat(req.query.length) < 0.0) {
    res.status(500)
    console.log(error)
  }
  fs.readdir(__dirname + '/uploads/', (err, files) => {
    files.map(file => {
      //console.log(file)
      count = count + parserfuncs.routesofLenJSON(__dirname + '/uploads/' + file, './gpx.xsd', parseFloat(req.query.length))
      //console.log(count)
    })
    res.send({
      count: count,
    })
  })
})

app.get('/tracksOfLen', async (req, res) => {
  var count = 0;
  if (parseFloat(req.query.length) < 0.0) {
    res.status(500)
    console.log(error)
  }
  fs.readdir(__dirname + '/uploads/', (err, files) => {
    files.map(file => {
      //console.log(file)
      count = count + parserfuncs.tracksofLenJSON(__dirname + '/uploads/' + file, './gpx.xsd', parseFloat(req.query.length))
      //console.log(count)
    })
    res.send({
      count: count,
    })
  })
})

app.post('/updateFile', async (req, res) => {
  var filename = req.headers.fn
  var component = req.headers.compname
  var newname = req.headers.newname
  var type = req.headers.type
  var ret;
  if (!filename || !component || !newname) {
    res.status(500)
    console.log(error)
  }
  if (type === 'route') {
    ret = parserfuncs.updateRouteName(newname, component, __dirname + '/uploads/' + filename, './gpx.xsd')
  }
  if (type === 'track') {
    ret = parserfuncs.updateTrackName(newname, component, __dirname + '/uploads/' + filename, './gpx.xsd')
  }
  if (ret) {
    res.send({
      message: 'Successfully updated',
    })
  } else {
    res.status(500)
    res.send({
      error: 'An error occurred',
    })
  }
})

app.post('/createFile', async (req, res) => {
  var creator = req.headers.creator
  var fileName = req.headers.fn
  var version = req.headers.ver
  if (!fileName || !creator || !version) {
    res.status(500)
    console.log(error)
  } else {
    if (!fileName.includes('.gpx')) { // concats .gpx when the file does not contain one
      fileName = fileName + '.gpx'
    }
    var gpx = `{"version":${version},"creator":${creator}}`
    var fileExists = true
    fs.readdir(__dirname + '/uploads/', (error, files) => {
      fileExists = files.includes(fileName)
      if (fileExists) {
        res.status(500);
        console.log("error")
      } else {
        var ret = parserfuncs.createDotGPX(gpx, __dirname + '/uploads/' + fileName, './gpx.xsd');
        if (ret) {
          res.send({
            message: "successfully created file",
          })
        } else {
          res.status(500)
          res.send({
            message: "server error: file was not created"
          })
        }
      }
    })
  }
})

app.listen(portNum);
console.log('Running app at localhost: ' + portNum);

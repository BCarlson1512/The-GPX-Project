// Put all onload AJAX calls here, and event listeners
jQuery(document).ready(function () {
    $('#updateCompContainer').hide()
    $('#docTable').hide();
    $('#editGPX').hide();
    $('#createRteInputs').hide();
    $('#rteTable').hide();
    $('#trkTable').hide();
    $('#queryForm').hide();
    $('#loggedInContainer').hide();

    /* 
    * this is awful practice... but since i dont want to 
    * setup dotenv or jwt auth, this is what im resorting to doing
    * for user authentication 
    * YOLO: CIS2750 Hackathon life :)
    */
    var loggedIn = false;
    var userName;
    var password;
    var hostName;
    var dbName;

    // clears the tables within the database
    function clearTables() {
        $.ajax({
            type: 'delete',
            dataType: 'json',
            url: '/clearDB',
            data: {
                username: userName,
                password: password,
                hostname: hostName,
                database: dbName,
            },
            success: (data) => {
                alert("MYSQL Database Successfully Cleared");
            },
            error: (data) => {
                alert(data.responseJSON.message)
            }
        })
    }

    // updates the database with new information
    function updateDB() {
        $.ajax({
            type: 'put',
            dataType: 'json',
            url: '/uploadDB',
            data: {
                username: userName,
                password: password,
                hostname: hostName,
                database: dbName,
            },
            success: (data) => {
                console.log("Success");
            },
            error: (data) => {
                console.log(data)
                //alert(data.responseJSON.message)
            }
        })
    }

    // generates query tables in html
    function updateDBTables(data) {
        console.log(data)
        let i = 0;
        var route_markup;
        var point_markup;
        var sortingType = $('#sortSelect').val()
        console.log(sortingType)
        $('.rteDbData').empty()
        $('.wptDbData').empty()
        if (data.queryType.includes('Pts')) {
            console.log("Points Query")
            data.data.map(point => {
                console.log(point)
                if (point.point_name === '') {
                    point.point_name = `Point ${i}`
                    i++
                }
                point_markup =
                    `<tr>
                <td>${point.route_id}</td>
                <td>${point.point_index}</td>
                <td>${point.point_name}</td>
                <td>${point.latitude}</td>
                <td>${point.longitude}</td>
                </tr>`;
                $('.wptDbData').append(point_markup)
            })
        }
        if (data.queryType.includes('Rte')) {
            if (sortingType === 'RteLen') { // sort by length
                data.data.sort((a, b) => {
                    let aLen = a.route_len;
                    let bLen = b.route_len;
                    if (aLen < bLen) return -1;
                    if (aLen > bLen) return 1;
                    return 0;
                })
            }
            if (sortingType === 'RteNam') { // sort by name
                data.data.sort((a, b) => {
                    let aLen = a.route_name;
                    let bLen = b.route_name;
                    if (aLen < bLen) return -1;
                    if (aLen > bLen) return 1;
                    return 0;
                })
            }
            console.log("Routes Query")
            data.data.map(route => {
                console.log(route)
                if (route.route_name === '') {
                    route.route_name = `Route ${i}`
                    i++
                }
                route_markup =
                    `<tr>
                <td>${route.gpx_id}</td>
                <td>${route.route_name}</td>
                <td>${route.route_len}</td>
                </tr>`;
                $('.rteDbData').append(route_markup)
            })
        }
    }

    // On page-load AJAX Example
    jQuery.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/endpoint1',   //The server endpoint we are connecting to
        data: {
            data1: "Value 1",
            data2: 1234.56
        },
        success: function (data) {
            /*  Do something with returned object
                Note that what we get is an object, not a string, 
                so we do not need to parse it on the server.
                JavaScript really does handle JSONs seamlessly
            */
            jQuery('#blah').html("On page load, received string '" + data.somethingElse + "' from server");
            //We write the object to the console to show that the request was successful
            console.log(data);
        },
        fail: function (error) {
            // Non-200 return, do something with error
            $('#blah').html("Server error");
        }
    });
    jQuery.ajax({
        type: 'get',
        dataType: 'json',
        url: '/uploads',
        success: function (data) {
            var resp = data.gpx;
            var names = data.names;
            var i = 0;
            //console.log(data.gpx)
            //console.log(names)
            if (data.gpx[0] !== null) {
                $('.fileData').empty()
            }
            $('#blah').html("Successfully loaded existing files");
            resp.forEach(data => {
                //console.log(data)
                if (data !== null) {
                    var markup =
                        `<tr>
                    <td><a href="${names[i]}">${names[i]}<a></td>
                    <td>${data.version}</td>
                    <td>${data.creator}</td>
                    <td>${data.numWaypoints}</td>
                    <td>${data.numRoutes}</td>
                    <td>${data.numTracks}</td>
                    </tr>`;
                    $('tbody.fileData').append(markup);
                }
                i++;
            })
            i = 0;
            var markup2;
            $('.fileAccess').empty()
            $('.createRteAccess').empty()
            $('.fileAccess').formSelect().append(`<option value="" disabled selected>Choose your file</option>`)
            $('.createRteAccess').formSelect().append(`<option value="" disabled selected>Choose your component</option>`)
            names.map(name => {
                //console.log(name)
                markup2 = `<option value="${name}">${name}</option>`;
                $('.fileAccess').formSelect().append(markup2);
                $('.createRteAccess').formSelect().append(markup2);
            })
            markup2
            resp.forEach(data => {

                i++;
            })
        },
        fail: function (error) {
            $('#blah').html("Server error");
            alert("Server error")
            console.log(error)
        }
    });
    $('.fileAccess').on("change", (e) => {
        e.preventDefault();
        var fileName = e.target.value;
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/detailedParse',
            data: {
                fn: fileName,
            },
            success: (data) => {
                $('#docTable').show();
                $('#editGPX').show();
                $('.docData').empty();
                $('.componentAccess').empty();
                $('.gpxDataAccess').empty();
                $('.gpxDataAccess').formSelect().append(`<option value="" disabled selected>Choose your component</option>`);
                $('.componentAccess').formSelect().append(`<option value="" disabled selected>Choose your component</option>`);
                $('#blah').html("Successfully Loaded File");
                console.log(data.parsedGPX[0]);
                var count = 1;
                var routes = data.parsedGPX[0].routes;
                var tracks = data.parsedGPX[0].tracks;
                var markup;
                var rteMarkup;
                var trkMarkup;
                $('.componentAccess').formSelect().append(`<optgroup label="Routes">`);
                $('.gpxDataAccess').formSelect().append(`<optgroup label="Routes">`);
                routes.forEach(route => {
                    markup =
                        `<tr>
                    <td>Route ${count}</td>
                    <td>${route.name}</td>
                    <td>${route.numPoints}</td>
                    <td>${route.len}m</td>
                    <td>${route.loop}</td>
                    </tr>`
                    rteMarkup = `<option class="route" value="${route.name}">Route ${count} - ${route.name}</option>`
                    count++;
                    $('.docData').append(markup);
                    $('.componentAccess').formSelect().append(rteMarkup);
                    $('.gpxDataAccess').formSelect().append(rteMarkup);
                })
                count = 1;
                $('.componentAccess').formSelect().append(`</optgroup>`);
                $('.componentAccess').formSelect().append(`<optgroup label="Tracks">`);
                $('.gpxDataAccess').formSelect().append(`</optgroup>`);
                $('.gpxDataAccess').formSelect().append(`<optgroup label="Tracks">`);
                tracks.forEach(track => {
                    markup =
                        `<tr>
                    <td>Track ${count}</td>
                    <td>${track.name}</td>
                    <td>${track.numPoints}</td>
                    <td>${track.len}m</td>
                    <td>${track.loop}</td>
                    </tr>`
                    trkMarkup = `<option class="track" value="${track.name}">Track ${count} - ${track.name}</option>`
                    $('.componentAccess').formSelect().append(trkMarkup);
                    $('.gpxDataAccess').formSelect().append(trkMarkup);
                    count++;
                    $('.docData').append(markup);
                })
                $('.componentAccess').formSelect().append(`</optgroup>`);
                $('.gpxDataAccess').formSelect().append(`</optgroup>`);
            },
            fail: (data) => {
                alert("Server error")
                console.log(error)
            }
        })

    })
    $('.gpxDataAccess').on('change', (e) => {
        e.preventDefault()
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/getGpxData',
            data: {
                fn: $('.fileAccess').val(),
                type: $('.gpxDataAccess :selected').attr('class'),
                name: $('.gpxDataAccess').val()
            },
            success: (data) => {
                console.log(data.gpxData)
                var displayStr = "";
                data.gpxData.forEach(gData => {
                    displayStr = displayStr + "\n" + gData.name + ": " + gData.value + "\n"
                })
                alert("Additional data for " + $('.gpxDataAccess').val() + ": " + displayStr);
            },
            fail: (error) => {
                alert("Server error" + error)
            }
        })
    })
    $('.componentAccess').on("change", (e) => {
        e.preventDefault()
        var compname = e.target.value;
        $('#updateCompContainer').show()
        $('#nameBox').attr('placeholder', `${e.target.value}`)
        $('#nameBox').attr('value', `${e.target.value}`)
    })
    $('#updateComponentForm').submit((e) => {
        e.preventDefault();
        $.ajax({
            type: 'post',
            dataType: 'json',
            url: '/updateFile',
            headers: {
                newName: $('#nameBox').val(),
                compName: $('.componentAccess').val(),
                fn: $('.fileAccess').val(),
                type: $('.componentAccess :selected').attr('class'),
            },
            success: (data) => {
                $('#editGPX').hide();
                if (loggedIn) {
                    clearTables();
                    updateDB();
                }
                alert("Component updated successfully")
            },
            fail: (error) => {
                console.log(error)
                alert("Server error: ")
            }
        })
        $('#updateCompContainer').hide()
    })
    $('#searchTrkPtsForm').submit((e) => {
        e.preventDefault()
        $('.trkData').empty()
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/tracksBetween',
            data: {
                startLat: $('#startTrkLatBox').val(),
                startLon: $('#startTrkLonBox').val(),
                destLat: $('#endTrkLatBox').val(),
                destLon: $('#endTrkLonBox').val(),
                delta: $('#trkDeltaBox').val(),
            },
            success: (data) => {
                $('#trkTable').show();
                var names = data.names;
                var tracks = data.tracks;
                var i = 0;
                var markup;
                tracks.forEach(track => {
                    track.map(tr => {
                        markup =
                            `<tr>
                        <td>${names[i]}</td>
                        <td>${tr.name}</td>
                        <td>${tr.len}</td>
                        <td>${tr.numPoints}</td>
                        <td>${tr.loop}</td> 
                        </tr>`
                        $('.trkData').append(markup)
                    })
                    i++;
                })
            },
            fail: (data) => {
                console.log(data)
                window.alert("Server error")
            }
        })
    })
    $('#searchRtePtsForm').submit((e) => {
        e.preventDefault()
        $('.rteData').empty()
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/routesBetween',
            data: {
                startLat: $('#startRteLatBox').val(),
                startLon: $('#startRteLonBox').val(),
                destLat: $('#endRteLatBox').val(),
                destLon: $('#endRteLonBox').val(),
                delta: $('#rteDeltaBox').val(),
            },
            success: (data) => {
                $('#rteTable').show();
                var names = data.names;
                var routes = data.routes;
                var i = 0;
                var markup;
                routes.forEach(route => {
                    if (names[i].includes('.gpx')) {
                        route.map(rt => {
                            markup =
                                `<tr>
                        <td>${names[i]}</td>
                        <td>${rt.name}</td>
                        <td>${rt.len}</td>
                        <td>${rt.numPoints}</td>
                        <td>${rt.loop}</td> 
                        </tr>`
                            $('.rteData').append(markup)
                        })
                    }

                    i++;
                })
            },
            fail: (data) => {
                alert("Server Error" + data.message)
            }
        })
    })
    $('#searchRteLenForm').submit((e) => {
        e.preventDefault()
        var len = $('#rteLenBox').val()
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/routesOfLen',
            data: {
                length: $('#rteLenBox').val(),
            },
            success: (data) => {
                console.log(data)
                if (data.count == 1) {
                    $('#rteLenPara').html(`${data.count} Route with length of ` + len + "m")
                } else if (data.count < 1) {
                    $('#rteLenPara').html(`No routes with length of ` + len + "m")
                } else {
                    $('#rteLenPara').html(`${data.count} Routes with length of ` + len + "m")
                }
            },
            fail: (data) => {
                alert("Server error " + data.message)
            }
        })
    })
    $('#searchTrkLenForm').submit((e) => {
        e.preventDefault()
        var len = $('#trkLenBox').val()
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/tracksOfLen',
            data: {
                length: $('#trkLenBox').val(),
            },
            success: (data) => {
                console.log(data)
                if (data.count == 1) {
                    $('#trkLenPara').html(`${data.count} Track with length of ` + len + "m")
                } else if (data.count < 1) {
                    $('#trkLenPara').html(`No tracks with length of ` + len + "m")
                } else {
                    $('#trkLenPara').html(`${data.count} Tracks with length of ` + len + "m")
                }
            },
            fail: (data) => {
                alert("Server error" + data.message)
            }
        })
    })
    $('#numWptBox').on("change", (e) => {
        e.preventDefault();
        $('.wptEntry').empty()
        var wptCount = parseInt(e.target.value);
        var markup;
        for (var i = 1; i <= wptCount; i++) {
            markup =
                `<div id="wptForm${i}" class="row">
                <div class="input-field col s6">
                    <label for="wptLatBox">Latitude</label>
                    <input type="text" class="form-control" id="wptLatBox${i}" name="wptLatBox" value="" required>
                </div>
                <div class="input-field col s6">
                    <label for="wptLonBox">Longitude</label>
                    <input type="text" class="form-control" id="wptLonBox${i}" name="wptLonBox" value="" required>
                </div>
            </div>`
            $('.wptEntry').append(markup)
        }
    })
    $('.createRteAccess').on("change", (e) => {
        e.preventDefault();
        $('#createRteInputs').show();
    })
    $('#createRouteForm').submit((e) => {
        e.preventDefault();
        var waypoints = [], route;
        var tmpLat, tmpLon;
        var numWpt = $('#numWptBox').val()
        for (var i = 1; i <= numWpt; i++) {
            tmpLat = $(`#wptLatBox${i}`).val()
            tmpLon = $(`#wptLonBox${i}`).val()
            tmp = `{"lat":${tmpLat},"lon":${tmpLon}}`
            if (i === 1) {
                waypoints = tmp
            } else {
                waypoints = waypoints + "-" + tmp
            }
        }
        route = `{"name":"${$('#rteNameBox').val()}","waypoints":${waypoints},}`
        $.ajax({
            type: 'post',
            dataType: 'json',
            url: '/createRoute',
            headers: {
                waypoints: waypoints,
                rtename: $('#rteNameBox').val(),
                fn: $('.createRteAccess').val(),
            },
            success: (data) => {
                console.log(data)
                if (loggedIn) {
                    clearTables();
                    updateDB();
                }
                alert(data.message)
                $('#createRteInputs').hide();
            },
            fail: (data) => {
                alert(data.message)
            },
        })
    })
    $('#createGPXForm').submit((e) => {
        //$('#blah').html("Form has data: "+$('#fnBox').val() +$('#creatorBox').val());
        e.preventDefault();
        $.ajax({
            type: 'post',
            dataType: 'json',
            url: '/createFile',
            headers: {
                fn: $('#fnBox').val(),
                creator: $('#creatorBox').val(),
                ver: 1.1
            },
            success: (data) => {
                if (loggedIn) {
                    clearTables();
                    updateDB();
                }
                alert("File created successfully")
            },
            fail: (error) => {
                alert("Server:" + error)
                console.log(error);
            }
        });
    })
    $('#storeFilesBtn').on("click", (e) => {
        e.preventDefault();
        if (!loggedIn) {
            alert("Not logged in... please sign in below")
        } else {
            $.ajax({
                type: 'put',
                dataType: 'json',
                url: '/uploadDB',
                data: {
                    username: userName,
                    password: password,
                    hostname: hostName,
                    database: dbName,
                },
                success: (data) => {
                    console.log(data)
                    alert("Successfully Stored files in MYSQL database")
                },
                error: (data) => {
                    console.log(data)
                    //alert(data.responseJSON.message)
                }
            })
        }
    })
    $('#clearDbBtn').on("click", (e) => {
        e.preventDefault();
        $.ajax({
            type: 'delete',
            dataType: 'json',
            url: '/clearDB',
            data: {
                username: userName,
                password: password,
                hostname: hostName,
                database: dbName,
            },
            success: (data) => {
                alert("MYSQL Database Successfully Cleared");
            },
            error: (data) => {
                alert(data.responseJSON.message)
            }
        })
    })
    $('#dbStatus').on("click", (e) => {
        e.preventDefault();
        if (!loggedIn) {
            alert("Not logged in... please sign in below")
        } else {
            $.ajax({
                type: 'get',
                dataType: 'json',
                url: '/dbStats',
                data: {
                    username: userName,
                    password: password,
                    hostname: hostName,
                    database: dbName,
                },
                success: (data) => {
                    console.log("Success")
                    console.log(data)
                    alert(data.message + "Number of Files: " + data.fileRows + "\nNumber of Routes: " + data.routeRows + "\nNumber of Points: " + data.pointRows)
                },
                error: (data) => {
                    alert(data.responseJSON.message)
                }
            })
        }
    })

    // pings the server for user signin...
    // again this isnt super secure but YOLO LOL
    $('#userLoginForm').submit((e) => {
        e.preventDefault();
        $.ajax({
            type: 'post',
            dataType: 'json',
            url: '/login',
            headers: {
                userName: $('#userNameBox').val(),
                password: $('#passwordBox').val(),
                hostname: $('#hostBox').val(),
                database: $('#dbBox').val(),
            },
            success: (data) => {
                loggedIn = true;
                $('#loggedInContainer').show();
                userName = $('#userNameBox').val();
                password = $('#passwordBox').val();
                hostName = $('#hostBox').val();
                dbName = $('#dbBox').val();
                alert(data.message + hostName);
                $('#loginContainer').hide();
            },
            fail: (error) => {
                console.log("failed")
                //alert(error.error.message)
                //console.log(error)
            },
            error: (data) => {
                alert(data.responseJSON.message)
                //console.log(data.responseJSON.message)
            }
        })
    })

    $('.querySelect').on("change", (e) => {
        e.preventDefault();
        $('#queryForm').show();
        var queryType = $('.querySelect').val();
        switch (queryType) {
            case 'AllRte':
                console.log("All Routes query");
                $('#queryRouteName').hide();
                $('#queryCount').hide();
                $('#queryFileName').hide();
                break;
            case 'SpecificRte':
                console.log("Query from a specific file")
                $('#queryRouteName').hide();
                $('#queryCount').hide();
                $('#queryFileName').show();
                break;
            case 'NShortRte':
                console.log("Shortest Routes");
                $('#queryRouteName').hide();
                $('#queryFileName').hide();
                $('#queryCount').show();
                break;
            case 'NLongRte':
                console.log("Longest Routes");
                $('#queryRouteName').hide();
                $('#queryFileName').hide();
                $('#queryCount').show();
                break;
            case 'SpecificPts':
                console.log("Points by Route")
                $('#queryRouteName').show();
                $('#queryFileName').hide();
                $('#queryCount').hide();
                break;
            case 'FilePts':
                console.log("Points by File")
                $('#queryRouteName').hide();
                $('#queryFileName').show();
                $('#queryCount').hide();
                break
            default:
                console.log("default")
                $('#queryForm').hide();
                break;
        }
    })

    $('#queryForm').submit((e) => {
        e.preventDefault();
        console.log($('.querySelect').val())
        console.log($('#routeCountBox').val())
        console.log($('#fileNameBox').val())
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/queryDB',
            data: {
                username: userName,
                password: password,
                hostname: hostName,
                database: dbName,
                queryType: $('.querySelect').val(),
                routeCount: $('#routeCountBox').val(),
                fileName: $('#fileNameBox').val(),
                routeName: $('#routeNameBox').val(),
            },
            success: (data) => {
                //console.log(data)
                updateDBTables(data)
                alert(data.message)
            },
            error: (data) => {
                alert(data.responseJSON.message)
            }
        })
    })

    $('.dropdown-trigger').dropdown({
        belowOrigin: false,
    });
    $('select').formSelect();
});

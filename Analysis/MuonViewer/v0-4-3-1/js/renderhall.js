// standard WebGL global variables
var container, scene, camera, renderer, controls;


// sensor geometry information: 1st index => detector id
//                              2nd index => geometry information object
var sensorGeoInfo;
var recGeoInfo;
var meshObjs = {};
var recMeshObjs;
var axisArr = [];

// trigger ID's for the selected muon
var curTrig;

// hall origin
var hallOrigin;

// trigger containers
var hallMuonId = new Array();
var litUpPmtSensors;
var litUpRpcSensors;


// set sensor frame color
var sensorFrameColor = {
  1:0xCCFFFF,
  2:0xFFCCFF,
  3:0xCCCCFF,
  4:0xEDDBFF,
  5:0xFFECD4,
  6:0xFEFFD4,
  7:0xD4FFE3
};
// set color of hit sensors
var sensorHitColor = {
  1:0xDD78FF,
  2:0xDD78FF,
  3:0xDD78FF,
  4:0xDD78FF,
  5:0xFFA500,
  6:0xFFFF00,
  7:0x7CFC00
};
// set color of reconstructed points
var recPtColor = {
  1:0x330033,
  2:0x330033,
  3:0x330033,
  4:0x330033,
  5:0xFF0000,
  6:0xCCCC00,
  7:0x339900
};

/// WebGL initialization
var viewwidth = $("#content").width();
var viewheight = $("#content").height();


// camera
camera = new THREE.PerspectiveCamera( 25, viewwidth / viewheight, 1, 100000 );
camera.position.z = 48000;

// axis arrows
// direction (normalized), origin, length, color(hex)
var direction = [];
direction.push(new THREE.Vector3(1,0,0));
direction.push(new THREE.Vector3(0,1,0));
direction.push(new THREE.Vector3(0,0,1));
var origin = new THREE.Vector3(0,0,7000);
var axisColor = [];
axisColor.push(0x0000FF);
axisColor.push(0xFF0000);
axisColor.push(0x00FF00);
for(var dd = 0; dd < 3; dd++)
  axisArr.push(new THREE.ArrowHelper(direction[dd], origin, 2000, axisColor[dd]));


// scene
scene = new THREE.Scene();

// renderer
renderer = new THREE.WebGLRenderer();
renderer.setSize( viewwidth, viewheight );

// draw the scene on the webpage
container = document.getElementById( "content" );
container.appendChild( renderer.domElement );

// controls
//controls = new THREE.OrbitControls( camera, renderer.domElement );
controls = new THREE.TrackballControls( camera, renderer.domElement );
/// specific to TrackballControls
controls.rotateSpeed = 3.0;
controls.staticMoving = true;
controls.dynamicDampingFactor = 0.3;
controls.zoomSpeed = 3;


// lighting
scene.add(new THREE.AmbientLight(0x555555));



// mark the sensors of the selected muon
$("#trigIdSelect").change(function(){
  var muId = $("#trigIdSelect").attr('value');
  
  getTrigInfoAndMark(muId);
  getRecInfoAndMark(muId);
  
  var dropdownidx = $("#trigIdSelect").prop("selectedIndex");
  /// if the navigation buttons reach the ends, disable them
  if(dropdownidx == 0) $("#prevmu").prop("disabled", true);
  else $("#prevmu").prop("disabled", false);
  if(dropdownidx == hallMuonId.length-1) $("#nextmu").prop("disabled", true);
  else $("#nextmu").prop("disabled", false);


  $("#runnum").val("");
  $("#filenum").val("");  
  if(curTrig.length)
  {
    $("#runnum").val(hallMuonId[dropdownidx].runNum);
    $("#filenum").val(hallMuonId[dropdownidx].fileNum);
    
    /// set date and time of the muon
    var datetime = hallMuonId[dropdownidx].date;
    var dateplustime = datetime.split(" ");
    $( "#datepicker" ).datepicker("setDate", dateplustime[0]);
    $("#timepicker").val(dateplustime[1]);
  }

  
  /// write trigger number to the texts
  var inputname;
  // clear all texts
  for(var i = 1; i <= 7; i++)
  {
    inputname = "input[name=tn" + i + "]";
    $(inputname).val("");
  }
  for(var i = 0; i < curTrig.length; i++)
  {
    inputname = "input[name=tn" + curTrig[i].detectorId + "]";
    $(inputname).val(curTrig[i].triggerNumber);
  }
  
  /// disable checkboxes not having reconstruction point
  for(var det = 1; det <= 7; det++)
  {
    var chboxid = "input[name=shrec" + det + "]";
    var disable = !(det in recMeshObjs);
    $(chboxid).prop('disabled', disable);
  }
  
  /// show/hide reconstruction points
  for(var det in recMeshObjs)
  {
    var chboxid = "input[name=shrec" + det + "]";
    $(chboxid).change();
  }
});



// retrieve information of detector sensors from database
// redraw the scene when hall changes
$("#hallSelect").change(function() {
  clearScene();
  
  console.log("number of objects in the scene: "+scene.children.length);
  var hallId = $("#hallSelect").attr("value");
  getHallOrigin(hallId);
  getSensorInfo(hallId);
  
  // attach sensors to the scene
  attachSensors(hallId);
  // attach coordinate axis to the scene
  attachCoordAxis();

  getHallMuons(hallId);
  writeMuonList();
  
  // wait for the geometry to set up
  window.setTimeout(function(){$("#trigIdSelect").change();}, 300);
  
  /// if hall 3, disable AD3&4
  if(hallId == 4)
  {
    $("input[name=shsen3]").prop('disabled', false);
    $("input[name=shsen4]").prop('disabled', false);
  }
  else
  {
    $("input[name=shsen3]").prop('disabled', true);
    $("input[name=shsen4]").prop('disabled', true);
  }

  /// reset all sensor and reconstruction checkboxes as show
  for(var det = 1; det <= 7; det++)
  {
    var chboxid = "input[name=shsen" + det + "]";
    $(chboxid).prop('checked', true);
    chboxid = "input[name=shrec" + det + "]";
    $(chboxid).prop('checked', true);
  }
  
});
$("#hallSelect").change();


/// place the date and time picker
$(function() {
  $( "#datepicker" ).datepicker({ dateFormat: "yy-mm-dd" });
});
/// endow panels with scrollbars
$(document).ready(function ($) {
  $('#menu').perfectScrollbar({
    wheelSpeed: 20,
    wheelPropagation: false
  });
  $('#submenu').perfectScrollbar({
    wheelSpeed: 20,
    wheelPropagation: false
  });
});



/// end of WebGL initialization



/// start animation
animate();



/// event handlers
// when the window is resized, resize the renderer accordingly
$( window ).resize(function() {
  var WIDTH = $("#content").width(),
      HEIGHT = $("#content").height();
  renderer.setSize(WIDTH, HEIGHT);
  camera.aspect = WIDTH / HEIGHT;
  camera.updateProjectionMatrix();
  
  $('#menu').perfectScrollbar('update');
  $('#submenu').perfectScrollbar('update');
});

// reload button
$("#reload").click(function() {
  getHallMuons($("#hallSelect").attr("value"));
  writeMuonList();
  $("#trigIdSelect").change();
});

// previous muon button
$("#prevmu").click(function() {
  var dropdownidx = $("#trigIdSelect").prop("selectedIndex");
  if(dropdownidx == 0) return;
  var dpval = hallMuonId[dropdownidx-1].muonId;
  $("#trigIdSelect").val(dpval).change();
});

// next muon button
$("#nextmu").click(function() {
  var dropdownidx = $("#trigIdSelect").prop("selectedIndex");
  if(dropdownidx == hallMuonId.length-1) return;
  var dpval = hallMuonId[dropdownidx+1].muonId;
  $("#trigIdSelect").val(dpval).change();
});

// clear button
$("#clear").click(function() {
  $("#runnum").val("");
  $("#filenum").val("");
  var inputname;
  // clear all texts
  for(var i = 1; i <= 7; i++)
  {
    inputname = "input[name=tn" + i + "]";
    $(inputname).val("");
  }
  $("#datepicker").val("");
  $("#timepicker").val("");
});

// find button
$("#find").click(function() {
  alert("hi");
});

/// sensor checkboxes
$("input[name=shsen1]").change(function(){
  for(var senId in meshObjs[1]) meshObjs[1][senId].visible = !meshObjs[1][senId].visible;
});
$("input[name=shsen2]").change(function(){
  for(var senId in meshObjs[2]) meshObjs[2][senId].visible = !meshObjs[2][senId].visible;
});
$("input[name=shsen3]").change(function(){
  for(var senId in meshObjs[3]) meshObjs[3][senId].visible = !meshObjs[3][senId].visible;
});
$("input[name=shsen4]").change(function(){
  for(var senId in meshObjs[4]) meshObjs[4][senId].visible = !meshObjs[4][senId].visible;
});
$("input[name=shsen5]").change(function(){
  for(var senId in meshObjs[5]) meshObjs[5][senId].visible = !meshObjs[5][senId].visible;
});
$("input[name=shsen6]").change(function(){
  for(var senId in meshObjs[6]) meshObjs[6][senId].visible = !meshObjs[6][senId].visible;
});
$("input[name=shsen7]").change(function(){
  for(var senId in meshObjs[7]) meshObjs[7][senId].visible = !meshObjs[7][senId].visible;
});
/// reconstruction checkboxes
$("input[name=shrec1]").change(function(){
  var visible = $("input[name=shrec1]").is(':checked');
  for(var kk = 0; kk < recMeshObjs[1].length; kk++) recMeshObjs[1][kk].visible = visible;
});
$("input[name=shrec2]").change(function(){
  var visible = $("input[name=shrec2]").is(':checked');
  for(var kk = 0; kk < recMeshObjs[2].length; kk++) recMeshObjs[2][kk].visible = visible;
});
$("input[name=shrec3]").change(function(){
  var visible = $("input[name=shrec3]").is(':checked');
  for(var kk = 0; kk < recMeshObjs[3].length; kk++) recMeshObjs[3][kk].visible = visible;
});
$("input[name=shrec4]").change(function(){
  var visible = $("input[name=shrec4]").is(':checked');
  for(var kk = 0; kk < recMeshObjs[4].length; kk++) recMeshObjs[4][kk].visible = visible;
});
$("input[name=shrec5]").change(function(){
  var visible = $("input[name=shrec5]").is(':checked');
  for(var kk = 0; kk < recMeshObjs[5].length; kk++) recMeshObjs[5][kk].visible = visible;
});
$("input[name=shrec6]").change(function(){
  var visible = $("input[name=shrec6]").is(':checked');
  for(var kk = 0; kk < recMeshObjs[6].length; kk++) recMeshObjs[6][kk].visible = visible;
});
$("input[name=shrec7]").change(function(){
  var visible = $("input[name=shrec7]").is(':checked');
  for(var kk = 0; kk < recMeshObjs[7].length; kk++) recMeshObjs[7][kk].visible = visible;
});

/// functions


// this function is executed on each animation frame
function animate(){
  requestAnimationFrame( animate );
  renderer.render( scene, camera );
  controls.update();
}



/// attach coordinate axis to the scene
function attachCoordAxis()
{
  for(var i = 0; i < 3; i++)
    scene.add(axisArr[i]);
}



function getHallMuons(hallId) {
  /// retrieve the muon belonging to the hall
  
  hallMuonId = [];
  
  var qrystr = 'SELECT * FROM muon WHERE hallId='+hallId;
  qrystr += ' ORDER BY muonId';
  
  $.ajax({
    async: false,
    type: 'POST',
    url: 'php/queryDatabase.php',
    data: {qrystr: qrystr},
    dataType: 'json',
    success: function(data) {
      $.each(data, function(i, row) {
        var obj = {
          muonId : row.muonId,
          runNum : row.runNumber,
          fileNum: row.fileNumber,
          date   : row.dateNTime
        };
        hallMuonId.push(obj);
      });
    },
    error: function(error) {
      alert(error);
    }
  });
}



function getHallOrigin(hallId) {
  /// query OWS origin and use as the hall origin
  
  var qrystr = 'SELECT detectorId, originX, originY, originZ';
  qrystr += ' FROM detector';
  qrystr += ' WHERE hallId=' + hallId;
  
  var detOrigs = [];
  
  $.ajax({
    async: false,
    type: 'POST',
    url: 'php/queryDatabase.php',
    data: {qrystr: qrystr},
    dataType: 'json',
    success: function(data) {
      $.each(data, function(i, row) {
        var obj = {
          x          : row.originX,
          y          : row.originY,
          z          : row.originZ
      };
      detOrigs[row.detectorId] = obj;
      });
      hallOrigin = detOrigs[6];
    },
    error: function(error) {
      alert(error);
    }
  });
}




function getRecInfoAndMark(muId) {
  recGeoInfo = {};
  
  /// remove the existing reconstructed points
  for(var det in recMeshObjs)
    for(var i = 0; i < recMeshObjs[det].length; i++)
      scene.remove(recMeshObjs[det][i]);

  recMeshObjs = {};
  
  for(var j = 0; j < curTrig.length; j++)
  {
    var det = curTrig[j].detectorId;
    var detTrigId = curTrig[j].detectorTriggerId;
    recGeoInfo[det] = [];
    
    if( det <= 4 ) /// get AD recon.
    {
      qrystr = 'SELECT * FROM recAdSimple';
      qrystr += ' WHERE detectorTriggerId=' + detTrigId;
      
      $.ajax({
        async: false,
        type: 'POST',
        url: 'php/queryDatabase.php',
        data: {qrystr: qrystr},
        dataType: 'json',
        success: function(data) {
          $.each(data, function(i, row) {
            var obj = {
              energy    : row.energy,
              x         : row.x,
              y         : row.y,
              z         : row.z
            };
            recGeoInfo[det].push(obj);
          });
        },
        error: function(error) {
          alert(error);
        }
      });
      
    }
    else if ( det <= 6 ) /// get WP recon.
    {
      
      qrystr = 'SELECT * FROM recPoolSimple';
      qrystr += ' WHERE detectorTriggerId=' + detTrigId;

      $.ajax({
        async: false,
        type: 'POST',
        url: 'php/queryDatabase.php',
        data: {qrystr: qrystr},
        dataType: 'json',
        success: function(data) {
          $.each(data, function(i, row) {
            var obj = {
              x         : row.x,
              y         : row.y,
              z         : row.z
            };
            recGeoInfo[det].push(obj);
          });
        },
        error: function(error) {
          alert(error);
        }
      });
      
    }
    else /// get RPC recon.
    {
      
      qrystr = 'SELECT * FROM recRpcSimple';
      qrystr += ' WHERE detectorTriggerId=' + detTrigId;
      
      $.ajax({
        async: false,
        type: 'POST',
        url: 'php/queryDatabase.php',
        data: {qrystr: qrystr},
        dataType: 'json',
        success: function(data) {
          $.each(data, function(i, row) {
            var obj = {
              x         : row.x,
              y         : row.y,
              z         : row.z
            };
            recGeoInfo[det].push(obj);
          });
        },
        error: function(error) {
          alert(error);
        }
      });
      
    }
  }
  
  
  /// create the meshes and add to the scene
  for(var det in recGeoInfo)
  {
    recMeshObjs[det] = [];

    for(var i = 0; i < recGeoInfo[det].length; i++)
    {
      var sphereGeo = new THREE.SphereGeometry( 200 );
      var material = new THREE.MeshBasicMaterial( { color: recPtColor[det], wireframe: false } );
      var mesh = new THREE.Mesh( sphereGeo, material );
      mesh.position.x = recGeoInfo[det][i].x - hallOrigin.x;
      mesh.position.y = recGeoInfo[det][i].y - hallOrigin.y;
      mesh.position.z = recGeoInfo[det][i].z - hallOrigin.z;
      recMeshObjs[det].push(mesh);
      
      scene.add(recMeshObjs[det][i]);
    }
  }
}




function getSensorInfo(hallId) {
  
  /// clear container array
  sensorGeoInfo = new Object();
  
  // AD information
  var nAd = 2;
  if(hallId > 2) nAd = 4;

  for(var j = 1; j <= nAd; j++)
  {
    sensorGeoInfo[j] = new Object();
    
    /// query PMT geometry information
    qrystr = 'SELECT *';
    qrystr += ' FROM adPmt';
    qrystr += ' WHERE detectorId=' + j;
    qrystr += ' AND hallId=' + hallId;
    
    $.ajax({
      async: false,
      type: 'POST',
      url: 'php/queryDatabase.php',
      data: {qrystr: qrystr},
      dataType: 'json',
      success: function(data) {
        $.each(data, function(i, row) {
          var obj = {
            hallId    : row.hallId,
            detectorId: row.detectorId,
            ring      : row.ring,
            col       : row.col,
            centerX   : row.centerX,
            centerY   : row.centerY,
            centerZ   : row.centerZ,
            normalX   : row.normalX,
            normalY   : row.normalY,
            normalZ   : row.normalZ
        };
        sensorGeoInfo[j][row.adPmtId] = obj;
        sensorGeoInfo[j][row.adPmtId].isHit = false;
        });
      },
      error: function(error) {
        alert(error);
      }
    });
  }
  
  
  // water pool information
  for(var k = 5; k <=6; k++)
  {
    /// clear container array
    sensorGeoInfo[k] = new Object();
    
    /// query PMT geometry information
    qrystr = 'SELECT *';
    qrystr += ' FROM wsPmt';
    qrystr += ' WHERE detectorId=' + k;
    qrystr += ' AND hallId=' + hallId;
    
    $.ajax({
      async: false,
      type: 'POST',
      url: 'php/queryDatabase.php',
      data: {qrystr: qrystr},
      dataType: 'json',
      success: function(data) {
        $.each(data, function(i, row) {
          var obj = {
            hallId      : row.hallId,
            detectorId  : row.detectorId,
            wallNumber  : row.wallNumber,
            wallSpot    : row.wallSpot,
            inwardFacing: row.inwardFacing,
            centerX     : row.centerX,
            centerY     : row.centerY,
            centerZ     : row.centerZ,
            normalX     : row.normalX,
            normalY     : row.normalY,
            normalZ     : row.normalZ
        };
        sensorGeoInfo[k][row.wsPmtId] = obj;
        sensorGeoInfo[k][row.wsPmtId].isHit = false;
        });
      },
      error: function(error) {
        alert(error);
      }
    });
  }
  
  
  // RPC information
  
  sensorGeoInfo[7] = new Object();
  
  /// query RPC geometry information
  qrystr = 'SELECT *';
  qrystr += ' FROM rpcStrip';
  qrystr += ' WHERE detectorId=7';
  qrystr += ' AND hallId=' + hallId;
  
  $.ajax({
    async: false,
    type: 'POST',
    url: 'php/queryDatabase.php',
    data: {qrystr: qrystr},
    dataType: 'json',
    success: function(data) {
      $.each(data, function(i, row) {
        var obj = {
          hallId      : row.hallId,
          detectorId  : row.detectorId,
          row         : row.row,
          col         : row.col,
          layer       : row.layer,
          strip       : row.strip,
          centerX     : row.centerX,
          centerY     : row.centerY,
          centerZ     : row.centerZ,
          normalX     : row.normalX,
          normalY     : row.normalY,
          normalZ     : row.normalZ
      };
      sensorGeoInfo[7][row.rpcStripId] = obj;
      sensorGeoInfo[7][row.rpcStripId].isHit = false;
      });
    },
    error: function(error) {
      alert(error);
    }
  });
}




function getTrigInfoAndMark(muId) {
  
  /// clear all sensors
  /// litUpPmtSensors is a 2d array. Key is pmtId and value is charge.
  /// litUpRpcSensors is a 1d array with rpcStripId as its value.
  litUpPmtSensors = [];
  litUpRpcSensors = [];
  
  
  /// clear previous lit up sensors
  for(var det = 1; det <= 7; det++)
  {
    for(var key in meshObjs[det])
    {
      meshObjs[det][key].material.wireframe = true;
      meshObjs[det][key].material.color.setHex(sensorFrameColor[det]);
      
      /// restore the PMT size
      if(det != 7)
        meshObjs[det][key].scale.set(100,100,100);
    }
  }
  
  /// clear the current trigger array
  curTrig = [];
  
  var qrystr = 'SELECT * FROM muonTriggerConstituent a INNER JOIN detectorTrigger b on a.detectorTriggerId=b.detectorTriggerId WHERE muonId=';
  qrystr += muId;

  
  $.ajax({
    async: false,
    type: 'POST',
    url: 'php/queryDatabase.php',
    data: {qrystr: qrystr},
    dataType: "json",
    success: function(data) {
      $.each(data, function(i, row) {
        var obj = {
          detectorTriggerId: row.detectorTriggerId,
          detectorId       : row.detectorId,
          triggerNumber    : row.triggerNumber,
          dateNTime        : row.dateNTime
        };
        curTrig.push(obj);
      });
    },
    error: function(error) {
      alert(error);
    }
  });


  /// get sensor information with trigger numbers
  for ( var j = 0; j < curTrig.length ; j++ )
  {

    if(curTrig[j].detectorId <= 4)
    {
      litUpPmtSensors[curTrig[j].detectorId] = [];
      /// query hit PMT data
      qrystr = 'SELECT adPmtId, totCharge';
      qrystr += ' FROM calibAdPmt';
      qrystr += ' WHERE detectorTriggerId=' + curTrig[j].detectorTriggerId;

    
      $.ajax({
        async: false,
        type: 'POST',
        url: 'php/queryDatabase.php',
        data: {qrystr: qrystr},
        dataType: 'json',
        success: function(data) {
          $.each(data, function(i, row) {
            litUpPmtSensors[curTrig[j].detectorId][row.adPmtId] = row.totCharge;
          });
        },
        error: function(error) {
          alert(error);
        }
      });
      
      /// find max charge
      var chArr = [];
      for( var key in litUpPmtSensors[curTrig[j].detectorId] )
        chArr.push(litUpPmtSensors[curTrig[j].detectorId][key]);
      
      var maxchg = Math.max.apply(Math, chArr);
      
      /// scale the hit PMTs
      for( var key in litUpPmtSensors[curTrig[j].detectorId] )
      {
        var curchg = litUpPmtSensors[curTrig[j].detectorId][key];
        var scalef = (1. + 1./maxchg * curchg)*100.;
        meshObjs[curTrig[j].detectorId][key].scale.set(scalef,scalef,scalef);
        meshObjs[curTrig[j].detectorId][key].material.color.setHex(sensorHitColor[curTrig[j].detectorId]);
      }

    }
    else if(curTrig[j].detectorId <= 6)
    {
      
      litUpPmtSensors[curTrig[j].detectorId] = {};
      
      /// query hit PMT data
      qrystr = 'SELECT wsPmtId, totCharge';
      qrystr += ' FROM calibWsPmt';
      qrystr += ' WHERE detectorTriggerId=' + curTrig[j].detectorTriggerId;

      $.ajax({
        async: false,
        type: 'POST',
        url: 'php/queryDatabase.php',
        data: {qrystr: qrystr},
        dataType: 'json',
        success: function(data) {
          $.each(data, function(i, row) {
            litUpPmtSensors[curTrig[j].detectorId][row.wsPmtId] = row.totCharge;
          })
        },
        error: function(error) {
          alert(error);
        }
      });
      
      /// find max charge
      var chArr = [];
      for( var key in litUpPmtSensors[curTrig[j].detectorId] )
        chArr.push(litUpPmtSensors[curTrig[j].detectorId][key]);
      
      var maxchg = Math.max.apply(Math, chArr);
      //if(curTrig[j].detectorId==6)
      //{
        //console.log(curTrig[j].detectorTriggerId);
        //console.log(litUpPmtSensors[curTrig[j].detectorId]);
      //}
      /// scale the hit PMTs
      for( var key in litUpPmtSensors[curTrig[j].detectorId] )
      {
        
        var curchg = litUpPmtSensors[curTrig[j].detectorId][key];
        var scalef = (1. + 1./maxchg * curchg)*100.;
        meshObjs[curTrig[j].detectorId][key].scale.set(scalef,scalef,scalef);
        meshObjs[curTrig[j].detectorId][key].material.color.setHex(sensorHitColor[curTrig[j].detectorId]);
      }
      
    }
    else
    {
      /// query hit strip data
      qrystr = 'SELECT rpcStripId';
      qrystr += ' FROM calibRpcStrip';
      qrystr += ' WHERE detectorTriggerId=' + curTrig[j].detectorTriggerId;
      
      $.ajax({
        async: false,
        type: 'POST',
        url: 'php/queryDatabase.php',
        data: {qrystr: qrystr},
        dataType: 'json',
        success: function(data) {
          $.each(data, function(i, row) {
            litUpRpcSensors.push(row.rpcStripId);
          });
        },
        error: function(error) {
          alert(error);
        }
      });
      for(var k = 0; k < litUpRpcSensors.length; k++) {
        meshObjs[7][litUpRpcSensors[k]].material.wireframe = false;
        meshObjs[7][litUpRpcSensors[k]].material.color.setHex(sensorHitColor[7]);
      }
      
    }

  }
  
}




function writeMuonList() {
  
  /// empty the muon ID list
  $("#trigIdSelect").empty();
  
  for(var i = 0; i < hallMuonId.length; i++) {
    var optstr = '<option value="'+hallMuonId[i].muonId+'">';
    optstr += hallMuonId[i].muonId+'</option>';
    $("#trigIdSelect").append(optstr);
  }
}




/// clear all geometry in the scene
function clearScene() {
  console.log("number of objects in the scene: "+scene.children.length);
  for ( var i = scene.children.length - 1; i >= 0 ; i -- )
    scene.remove(scene.children[ i ]);
}



function attachSensors(hallId) {
  
  // clear meshes in the scene
  meshObjs = {};
  
  // attach PMT sensors
  var loader = new THREE.JSONLoader();

  loader.load('js/PmtModel.js', function(geometry){
    
    for(var det = 1; det < 7; det++)
    {
      meshObjs[det] = {};
      
      for(var key in sensorGeoInfo[det])
      {
        if(sensorGeoInfo[det].hasOwnProperty(key)) {
          
          var pmtMaterial  = new THREE.MeshBasicMaterial( { color: sensorFrameColor[det], wireframe: true, wireframe_linewidth: 1 } );
          
          meshObjs[det][key] = new THREE.Mesh( geometry, pmtMaterial );
          meshObjs[det][key].scale.set( 100, 100, 100 );
          
          meshObjs[det][key].position.x = sensorGeoInfo[det][key].centerX - hallOrigin.x;
          meshObjs[det][key].position.y = sensorGeoInfo[det][key].centerY - hallOrigin.y;
          meshObjs[det][key].position.z = sensorGeoInfo[det][key].centerZ - hallOrigin.z;
          
          /// rotate PMTs
          var axis = new THREE.Vector3();
          var dir0 = new THREE.Vector3(0,1,0);
          var ez = new THREE.Vector3(0,0,1);
          var dirDest = new THREE.Vector3(sensorGeoInfo[det][key].normalX,sensorGeoInfo[det][key].normalY,sensorGeoInfo[det][key].normalZ);
          axis.crossVectors(dir0,dirDest);
          axis.normalize();
          
          var rotationAngle = Math.acos(dir0.dot(dirDest));
          if(axis.dot(ez) < 0) rotationAngle = -rotationAngle;
          
          if(Math.abs(sensorGeoInfo[det][key].normalZ) < 0.5)
            meshObjs[det][key].rotation.set(0,0,rotationAngle);
          else if(sensorGeoInfo[det][key].normalZ > 0.5)
            meshObjs[det][key].rotation.set(Math.PI/2,0,0);
          else if(sensorGeoInfo[det][key].normalZ < -0.5)
            meshObjs[det][key].rotation.set(-Math.PI/2,0,0);
          
          scene.add( meshObjs[det][key] );
        }
      }
    }

  });
  
  
  // attach RPC sensors
  
  /// clear mesh array
  meshObjs[7] = {};
  
  /// local to global rotation angle
  var l2gRotAngle = {
    1: 2.145009114803047,
    2: -1.3912024468683883,
    4: -2.6275930112560291
  };
  
  /// RPC geometry parameters
  var rpcStripGeometry = new THREE.CubeGeometry( 2080, 260, 2 );

  for(var key in sensorGeoInfo[7]) {
    if(sensorGeoInfo[7].hasOwnProperty(key)) {

      rpcMaterial  = new THREE.MeshBasicMaterial( { color: sensorFrameColor[7], wireframe: true } );
      //rpcMaterial  = new THREE.MeshBasicMaterial( { transparent: true, opacity: 0.5, color: sensorFrameColor[7], wireframe: false } );
      meshObjs[7][key] = new THREE.Mesh( rpcStripGeometry, rpcMaterial );
      meshObjs[7][key].position.x = sensorGeoInfo[7][key].centerX - hallOrigin.x;
      meshObjs[7][key].position.y = sensorGeoInfo[7][key].centerY - hallOrigin.y;
      meshObjs[7][key].position.z = sensorGeoInfo[7][key].centerZ - hallOrigin.z;
      
      /// rotate RPC strips
      var rotAngle;
      if(sensorGeoInfo[7][key].layer == 1 || sensorGeoInfo[7][key].layer == 4)
        rotAngle = 90.*Math.PI/180.+l2gRotAngle[hallId];
      else
        rotAngle = l2gRotAngle[hallId];
      var ez = new THREE.Vector3(0, 0, 1);
      var normal = new THREE.Vector3(sensorGeoInfo[7][key].normalX, sensorGeoInfo[7][key].normalY, sensorGeoInfo[7][key].normalZ);
      var tiltAngle = Math.acos(ez.dot(normal));
      var axis = new THREE.Vector3();
      axis.crossVectors(ez,normal);
      axis.normalize();
      
      var tilt = new THREE.Matrix4().makeRotationAxis(axis, tiltAngle);
      var rotation = new THREE.Matrix4().makeRotationAxis(ez, rotAngle);
      var totalRotation = new THREE.Matrix4();
      totalRotation.multiplyMatrices(tilt, rotation);
      
      //var eulerAngles = new THREE.Vector3();
      //eulerAngles.setEulerFromRotationMatrix(totalRotation);
      var eulerAngles = new THREE.Euler().setFromRotationMatrix(totalRotation);
      
      meshObjs[7][key].rotation.set(eulerAngles.x,eulerAngles.y,eulerAngles.z);
      
      scene.add( meshObjs[7][key] );
    }
  }
}

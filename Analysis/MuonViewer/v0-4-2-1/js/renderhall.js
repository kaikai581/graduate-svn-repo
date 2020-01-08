/// container definition

var validMuonId = new Array();
var curTrig = new Array();

var detOrigin;

/// avoid retrieving the geometry each time
/// each detector is dealt separately
var lastHall = 0;
var hallOrigin;

var calibAd = new Object();
var calibWs = new Object();
var calibRpc;

/// variables for WebGL rendering
var container;
var camera, scene, renderer, controls;
var adMaterial, wsMaterial, rpcMaterial;

var detectorElementMesh = new Array();


$(document).ready(function() {

  /// hide unnecessary forms
  $(".checkboxgroup").hide();


  /// get all trigger IDs in the database
  getMuonId();
  /// insert the valid ID into the drop down list
  for(var i = 0; i < validMuonId.length; i++) {
    var optstr = '<option value="'+validMuonId[i].muonId+'">';
    optstr += validMuonId[i].muonId+'</option>';
    $("#trigIdSelect").append(optstr);
  }

  /// process selected event
  $("#trigIdSelect").change(function(){
    
    /// clear trigger array
    curTrig = [];
    
    //var triggerId = $("#trigIdSelect").val();
    /// use .attr('value') instead of .val() to avoid a firefox warning message
    var muId = $("#trigIdSelect").attr('value');
    
    /// get the selected trigger
    getTriggeredDetectors(muId);
    
    /// if hall changes, deploy geometry
    /// otherwise clear all PMTs
    if(curTrig.length)
    {
      if(lastHall != curTrig[0].hallId)
      {
        deployGeometry(curTrig[0].hallId);
        lastHall = curTrig[0].hallId;
      }
      else resetDetectors(curTrig[0].hallId);
    }
    
    for ( var i = 0; i < curTrig.length ; i ++ )
    {
      
      /// start processing different detectors
      if(curTrig[i].detectorId <= 4)
        processAdTrigger(curTrig[i]);
      if(curTrig[i].detectorId == 5 || curTrig[i].detectorId == 6)
        processWsTrigger(curTrig[i]);
      if(curTrig[i].detectorId == 7)
        processRpcTrigger(curTrig[i]);
    }
    
    //scene = new THREE.Scene();
    
    /// clear WebGL scene
    //clearScene();

    init();
    
    animate();

  });
  
  /// manually trigger the event for initialization
  /// it has to be called later than the definition of the event handler
  $("#trigIdSelect").change();
});



function animate() {

  requestAnimationFrame( animate );

  controls.update();
  
  render();
}



function calculateColorFromCharge(minch, maxch, curch) {
    
  var colorstops = new Array();
  
  /// rainbow colors from purple to red
  colorstops.push(0x800080);
  colorstops.push(0x4B0080);
  colorstops.push(0x0000FF);
  colorstops.push(0x008000);
  colorstops.push(0xFFFF00);
  colorstops.push(0xFFA500);
  colorstops.push(0xFF0000);
  
  var nstops = colorstops.length;
  /// charge interval
  var chIntv = (maxch - minch) / (nstops - 1);
  
  var idx = Math.floor((curch - minch) / chIntv);
  
  /// if current color is the largest charge, return red
  if(idx == nstops - 1) return colorstops[nstops - 1];
  
  var startColor = new THREE.Vector3(colorstops[idx]>>16, (colorstops[idx]&0x00FF00)>>8, colorstops[idx]&0x0000FF);
  var endColor = new THREE.Vector3(colorstops[idx+1]>>16, (colorstops[idx+1]&0x00FF00)>>8, colorstops[idx+1]&0x0000FF);
  
  var curColor = startColor.add(endColor.sub(startColor).multiplyScalar((curch-minch)/chIntv-idx));
  
  var r = Math.round(curColor.x);
  var g = Math.round(curColor.y);
  var b = Math.round(curColor.z);
  
  return (r<<16)+(g<<8)+b;
}



function clearScene() {
  
  for ( var i = scene.children.length - 1; i >= 0 ; i -- )
    scene.remove(scene.children[ i ]);
  
  //detectorElementMesh = [];
  
  $('#container').empty();
  
}



function deployGeometry(hallId)
{
  /// query IWS origin and use as the hall origin
  var qrystr = 'SELECT originX, originY, originZ';
  qrystr += ' FROM detector';
  qrystr += ' WHERE detectorId=5'
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
          x          : row.originX,
          y          : row.originY,
          z          : row.originZ
      };
      hallOrigin = obj;
      })
    },
    error: function(error) {
      alert(error);
    }
  });
  
  
  /// deploy ADs
  var nAd;
  if(hallId <= 2) nAd = 2;
  else nAd = 4;
  
  for(var j = 1; j <= nAd; j++)
  {
    /// clear container array
    calibAd[j] = new Object();
    
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
        calibAd[j][row.adPmtId] = obj;
        calibAd[j][row.adPmtId].isHit = false;
        })
      },
      error: function(error) {
        alert(error);
      }
    });
  }
  
  
  /// deploy water pool
  for(var k = 5; k <= 6; k++)
  {
    /// clear container array
    calibWs[k] = new Object();
    
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
        calibWs[k][row.wsPmtId] = obj;
        calibWs[k][row.wsPmtId].isHit = false;
        })
      },
      error: function(error) {
        alert(error);
      }
    });
  }
  
  
  /// deploy RPC
  
  /// clear container array
  calibRpc = new Object();
  
  /// query PMT geometry information
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
      calibRpc[row.rpcStripId] = obj;
      calibRpc[row.rpcStripId].isHit = false;
      })
    },
    error: function(error) {
      alert(error);
    }
  });
  /// end of deploy RPC
  
}



function attachAd(det)
{
  
  var loader = new THREE.JSONLoader();

  loader.load('js/PmtModel.js', function(geometry){
    
    /* calculate mesh color with linear gradient of rainbow */
    /* small charge to cold color and large charge to hot color */
    var first = true;
    var chgmin, chgmax;
    var keymin, keymax;
    
    for(var key in calibAd[det]) {
      if(calibAd[det].hasOwnProperty(key)) {
        if(!calibAd[det][key].isHit) continue;

        /// find the smallest and largest charges
        if(first) {
          chgmin = calibAd[det][key].totCharge;
          keymin = key;
          chgmax = calibAd[det][key].totCharge;
          keymax = key;
          first = false;
        }
        else {
          if(calibAd[det][key].totCharge - chgmin < 0) {
            chgmin = calibAd[det][key].totCharge;
            keymin = key;
          }
          if(calibAd[det][key].totCharge - chgmax > 0) {
            chgmax = calibAd[det][key].totCharge;
            keymax = key;
          }
        }
      }
    }

    
    for(var key in calibAd[det]) {
      if(calibAd[det].hasOwnProperty(key)) {
        
        var curColor = 0xFFFDD0;
        var frameType = true;
        if(calibAd[det][key].isHit) {
          curColor = calculateColorFromCharge(chgmin, chgmax, calibAd[det][key].totCharge);
          frameType = false;
        }
        
        adMaterial  = new THREE.MeshBasicMaterial( { color: curColor, wireframe: frameType } );
        
        var mesh = new THREE.Mesh( geometry, adMaterial );
        if(Math.abs(calibAd[det][key].normalZ) < 0.5)
          mesh.scale.set( 100, 100, 100 );
        else
          mesh.scale.set( 25, 25, 25 );
        
        mesh.position.x = calibAd[det][key].centerX - hallOrigin.x;
        mesh.position.y = calibAd[det][key].centerY - hallOrigin.y;
        mesh.position.z = calibAd[det][key].centerZ - hallOrigin.z;
        
        /// rotate AD PMTs
        var axis = new THREE.Vector3();
        var dir0 = new THREE.Vector3(0,1,0);
        var ez = new THREE.Vector3(0,0,1);
        var dirDest = new THREE.Vector3(calibAd[det][key].normalX,calibAd[det][key].normalY,calibAd[det][key].normalZ);
        axis.crossVectors(dir0,dirDest);
        axis.normalize();
        
        var rotationAngle = Math.acos(dir0.dot(dirDest));
        if(axis.dot(ez) < 0) rotationAngle = -rotationAngle;
        
        if(Math.abs(calibAd[det][key].normalZ) < 0.5)
          mesh.rotation.set(0,0,rotationAngle);
        else if(calibAd[det][key].normalZ > 0.5)
          mesh.rotation.set(Math.PI/2,0,0);
        else if(calibAd[det][key].normalZ < -0.5)
          mesh.rotation.set(-Math.PI/2,0,0);
        
        mesh.matrix.setRotationFromEuler(mesh.rotation); // Set initial rotation
        
        scene.add( mesh );
      }
    }
  });
  
}



function attachRpc()
{
  /// local to global rotation angle
  var l2gRotAngle = {
    1: 2.145009114803047,
    2: -1.3912024468683883,
    4: -2.6275930112560291
  }
  
  /// RPC geometry parameters
  var rpcStripGeometry = new THREE.CubeGeometry( 2080, 260, 2 );

  for(var key in calibRpc) {
    if(calibRpc.hasOwnProperty(key)) {
      
      var frameType = true;
      if(calibRpc[key].isHit) frameType = false;

      rpcMaterial  = new THREE.MeshBasicMaterial( { color: 0x0055ff, wireframe: frameType } );
      var mesh = new THREE.Mesh( rpcStripGeometry, rpcMaterial );
      mesh.position.x = calibRpc[key].centerX - hallOrigin.x;
      mesh.position.y = calibRpc[key].centerY - hallOrigin.y;
      mesh.position.z = calibRpc[key].centerZ - hallOrigin.z;
      
      /// rotate RPC strips
      var rotAngle;
      if(calibRpc[key].layer == 1 || calibRpc[key].layer == 4)
        rotAngle = 90.*Math.PI/180.+l2gRotAngle[curTrig[0].hallId];
      else
        rotAngle = l2gRotAngle[curTrig[0].hallId];
      var ez = new THREE.Vector3(0, 0, 1);
      var normal = new THREE.Vector3(calibRpc[key].normalX, calibRpc[key].normalY, calibRpc[key].normalZ);
      var tiltAngle = Math.acos(ez.dot(normal));
      var axis = new THREE.Vector3();
      axis.crossVectors(ez,normal);
      axis.normalize();
      
      var tilt = new THREE.Matrix4().makeRotationAxis(axis, tiltAngle);
      var rotation = new THREE.Matrix4().makeRotationAxis(ez, rotAngle);
      var totalRotation = new THREE.Matrix4();
      totalRotation.multiplyMatrices(tilt, rotation);
      
      var eulerAngles = new THREE.Vector3();
      eulerAngles.setEulerFromRotationMatrix(totalRotation);
      
      mesh.rotation.set(eulerAngles.x,eulerAngles.y,eulerAngles.z);
      mesh.matrix.setRotationFromEuler(mesh.rotation);
      
      scene.add( mesh );
    }
  }
}



function attachWs(det)
{
  
  var loader = new THREE.JSONLoader();

  loader.load('js/PmtModel.js', function(geometry){
    
    /* calculate mesh color with linear gradient of rainbow */
    /* small charge to cold color and large charge to hot color */
    var first = true;
    var chgmin, chgmax;
    var keymin, keymax;
    
    for(var key in calibWs[det]) {
      if(calibWs[det].hasOwnProperty(key)) {
        if(!calibWs[det][key].isHit) continue;

        /// find the smallest and largest charges
        if(first) {
          chgmin = calibWs[det][key].totCharge;
          keymin = key;
          chgmax = calibWs[det][key].totCharge;
          keymax = key;
          first = false;
        }
        else {
          if(calibWs[det][key].totCharge - chgmin < 0) {
            chgmin = calibWs[det][key].totCharge;
            keymin = key;
          }
          if(calibWs[det][key].totCharge - chgmax > 0) {
            chgmax = calibWs[det][key].totCharge;
            keymax = key;
          }
        }
      }
    }

    
    for(var key in calibWs[det]) {
      if(calibWs[det].hasOwnProperty(key)) {
        
        var curColor = 0xCCCCFF;
        var frameType = true;
        if(calibWs[det][key].isHit) {
          curColor = calculateColorFromCharge(chgmin, chgmax, calibWs[det][key].totCharge);
          frameType = false;
        }
        
        wsMaterial  = new THREE.MeshBasicMaterial( { color: curColor, wireframe: frameType } );
        
        var mesh = new THREE.Mesh( geometry, wsMaterial );
        mesh.scale.set( 100, 100, 100 );
        
        mesh.position.x = calibWs[det][key].centerX - hallOrigin.x;
        mesh.position.y = calibWs[det][key].centerY - hallOrigin.y;
        mesh.position.z = calibWs[det][key].centerZ - hallOrigin.z;
        
        /// rotate WS PMTs
        var axis = new THREE.Vector3();
        var dir0 = new THREE.Vector3(0,1,0);
        var ez = new THREE.Vector3(0,0,1);
        var dirDest = new THREE.Vector3(calibWs[det][key].normalX,calibWs[det][key].normalY,calibWs[det][key].normalZ);
        axis.crossVectors(dir0,dirDest);
        axis.normalize();
        
        var rotationAngle = Math.acos(dir0.dot(dirDest));
        if(axis.dot(ez) < 0) rotationAngle = -rotationAngle;
        
        if(Math.abs(calibWs[det][key].normalZ) < 0.5)
          mesh.rotation.set(0,0,rotationAngle);
        else if(calibWs[det][key].normalZ > 0.5)
          mesh.rotation.set(Math.PI/2,0,0);
        else if(calibWs[det][key].normalZ < -0.5)
          mesh.rotation.set(-Math.PI/2,0,0);
        
        mesh.matrix.setRotationFromEuler(mesh.rotation); // Set initial rotation
        
        scene.add( mesh );
      }
    }
  });
  
}



function getTriggeredDetectors(muId) {
  
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
          hallId           : row.hallId,
          detectorId       : row.detectorId,
          triggerNumber    : row.triggerNumber,
          dateNTime        : row.dateNTime
        };
        curTrig.push(obj);
      })
    },
    error: function(error) {
      alert(error);
    }
  });
  
}


function getMuonId() {
  
  var qrystr = 'SELECT muonId FROM muon ORDER BY muonId';
  
  $.ajax({
    async: false,
    type: 'POST',
    url: 'php/queryDatabase.php',
    data: {qrystr: qrystr},
    dataType: 'json',
    success: function(data) {
      $.each(data, function(i, row) {
        var obj = {
          muonId: row.muonId
        };
        validMuonId.push(obj);
      })
    },
    error: function(error) {
      alert(error);
    }
  });
}



function init() {

  camera = new THREE.PerspectiveCamera( 25, window.innerWidth / window.innerHeight, 1, 100000 );
  
  //camera = new THREE.PerspectiveCamera( 25, 800 / 480, 1, 100000 );
  
  camera.position.z = 48000;
  
  scene = null;
  scene = new THREE.Scene();
  clearScene();
  
  var ambientLight = new THREE.AmbientLight(0x555555);
  scene.add(ambientLight);
  
  /// add the detector geometry to the scene
  attachAd(1);
  attachAd(2);
  if(curTrig[0].hallId > 2)
  {
    attachAd(3);
    attachAd(4);
  }
  attachWs(5);
  attachWs(6);
  attachRpc();
  
  renderer = new THREE.WebGLRenderer();
  
  //renderer.setSize( window.innerWidth, window.innerHeight );
  
  //var docheight = document.documentElement.offsetHeight;
  var docheight = window.innerHeight-20;
  
  renderer.setSize( docheight*window.innerWidth/window.innerHeight, docheight );
  
  //container = document.getElementById( 'arena' );
  
  container = document.getElementById( 'container' );
  
  container.appendChild( renderer.domElement );
  
  controls = new THREE.TrackballControls( camera, renderer.domElement );
  
  controls.rotateSpeed = 1.0;
  controls.zoomSpeed = 5;
  controls.panSpeed = 0.8;
  controls.noZoom = false;
  controls.noPan = false;
  controls.staticMoving = true;
  controls.dynamicDampingFactor = 0.3;
  controls.keys = [ 65, 83, 68 ];
  controls.addEventListener( 'change', render );
  
  /// create a point light
  var pointLight = new THREE.PointLight(0xFFFFFF);
  
  /// set its position
  pointLight.position.x = 10;
  pointLight.position.y = 10;
  pointLight.position.z = 130;
  
  /// add to the scene
  scene.add(pointLight);

  window.addEventListener( 'resize', onWindowResize, false );
}



function onWindowResize() {
  
  camera.aspect = window.innerWidth / window.innerHeight;
  camera.updateProjectionMatrix();
  
  renderer.setSize( window.innerWidth, window.innerHeight );
  
  controls.handleResize();
  
  render();
  
}



function processAdTrigger(trig) {

  /// query hit PMT data
  qrystr = 'SELECT adPmtId, totCharge';
  qrystr += ' FROM calibAdPmt';
  qrystr += ' WHERE detectorTriggerId=' + trig.detectorTriggerId;

  $.ajax({
    async: false,
    type: 'POST',
    url: 'php/queryDatabase.php',
    data: {qrystr: qrystr},
    dataType: 'json',
    success: function(data) {
      $.each(data, function(i, row) {
        calibAd[trig.detectorId][row.adPmtId].isHit = true;
        calibAd[trig.detectorId][row.adPmtId].totCharge = row.totCharge;
      })
    },
    error: function(error) {
      alert(error);
    }
  });


  /// query hit PMT TDC data
  /*for(var key in calibAd) {
    if(calibAd.hasOwnProperty(key)) {
      qrystr = 'SELECT calibAdPmtTdc.time_ps, calibAdPmtTdc.charge';
      qrystr += ' FROM calibAdPmtTdc';
      qrystr += ' INNER JOIN calibAdPmt';
      qrystr += ' ON calibAdPmtTdc.calibAdPmtId = calibAdPmt.calibAdPmtId';
      qrystr += ' WHERE detectorTriggerId=' + triggerId;
      qrystr += ' AND adPmtId=' + key;
      
      $.ajax({
        async: false,
        type: 'POST',
        url: 'php/queryDatabase.php',
        data: {qrystr: qrystr},
        dataType: 'json',
        success: function(data) {
          $.each(data, function(i, row) {
            var obj = {
              time  : row.time_ps/1000.,
              charge: row.charge
            };
            calibAd[key].tdc = new Array();
            calibAd[key].tdc.push(obj);
          })
        },
        error: function(error) {
          alert(error);
        }
      });
    }
  }*/


  //for(var key in calibAd) {
    //if(calibAd.hasOwnProperty(key)) {
      //if(calibAd[key].isHit)
        //$("#container").html(key+' '+calibAd[key].tdc[0].charge);
    //}
  //}
  //$("#container").html(calibAd.length);
  //$("#container").html(qrystr);
  //$("#container").html(calibAd[calibAd.length-1].centerX);
}


function processRpcTrigger(trig) {
  /// query hit strip data
  qrystr = 'SELECT rpcStripId';
  qrystr += ' FROM calibRpcStrip';
  qrystr += ' WHERE detectorTriggerId=' + trig.detectorTriggerId;

  $.ajax({
    async: false,
    type: 'POST',
    url: 'php/queryDatabase.php',
    data: {qrystr: qrystr},
    dataType: 'json',
    success: function(data) {
      $.each(data, function(i, row) {
        calibRpc[row.rpcStripId].isHit = true;
      })
    },
    error: function(error) {
      alert(error);
    }
  });
}


function processWsTrigger(trig) {
  
  /// query hit PMT data
  qrystr = 'SELECT wsPmtId, totCharge';
  qrystr += ' FROM calibWsPmt';
  qrystr += ' WHERE detectorTriggerId=' + trig.detectorTriggerId;
  
  //for(var key in calibWs[trig.detectorId])
    //if(calibWs[trig.detectorId].hasOwnProperty(key))
      //calibWs[trig.detectorId][key].isHit = false;

  $.ajax({
    async: false,
    type: 'POST',
    url: 'php/queryDatabase.php',
    data: {qrystr: qrystr},
    dataType: 'json',
    success: function(data) {
      $.each(data, function(i, row) {
        calibWs[trig.detectorId][row.wsPmtId].isHit = true;
        calibWs[trig.detectorId][row.wsPmtId].totCharge = row.totCharge;
      })
    },
    error: function(error) {
      alert(error);
    }
  });
}



function render() {
  renderer.render( scene, camera );
}



function resetDetectors(hall)
{
  var nAd;
  
  if(hall > 2) nAd = 4;
  else nAd = 2;
  
  for(var i = 1; i <= nAd; i++)
  {
    for(var key in calibAd[i])
      if(calibAd[i].hasOwnProperty(key))
        calibAd[i][key].isHit = false;
  }
  
  for(var i = 5; i <= 6; i++)
  {
    for(var key in calibWs[i])
      if(calibWs[i].hasOwnProperty(key))
        calibWs[i][key].isHit = false;
  }
  
  for(var key in calibRpc)
    if(calibRpc.hasOwnProperty(key))
      calibRpc[key].isHit = false;
}

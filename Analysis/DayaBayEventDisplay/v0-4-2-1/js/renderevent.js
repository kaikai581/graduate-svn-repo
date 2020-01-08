/// container definition

var validTrigId = new Array();
var curTrig = new Array();

var detOrigin;

var calibAd = new Object();

/// variables for WebGL rendering
var container;
var camera, scene, renderer, controls;
var pmtGeometry, adMaterial;

var detectorElementMesh = new Array();

$(document).ready(function() {

  /// hide unnecessary forms
  $(".checkboxgroup").hide();

  /// get all trigger IDs in the database
  getTriggerId();
  /// insert the valid ID into the drop down list
  for(var i = 0; i < validTrigId.length; i++) {
    var optstr = '<option value="'+validTrigId[i].detectorTriggerId+'">';
    optstr += validTrigId[i].detectorTriggerId+'</option>';
    $("#trigIdSelect").append(optstr);
  }

  /// process selected event
  $("#trigIdSelect").change(function(){
    
    /// clear trigger array
    curTrig = [];
    
    //var triggerId = $("#trigIdSelect").val();
    /// use .attr('value') instead of .val() to avoid a firefox warning message
    var triggerId = $("#trigIdSelect").attr('value');
    
    /// get the selected trigger
    getTrigger(triggerId);
    
    if(curTrig.length != 1) {
      alert("trigger doesn't exist");
      return -1;
    }
    
    if(curTrig[0].detectorId <= 4)
      processAdTrigger(triggerId);
    if(curTrig[0].detectorId == 5 || curTrig[0].detectorId == 6)
      processWsTrigger();
    if(curTrig[0].detectorId == 7)
      processRpcTrigger();
    
    scene = new THREE.Scene();
    
    /// clear WebGL scene
    clearScene();
    
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
  
  detectorElementMesh = [];
  
  $('#container').empty();
  
}



function attachAd()
{
  
  pmtGeometry = new THREE.SphereGeometry( 100, 8, 8 );
  
  //for(var key in calibAd) {
    //if(calibAd.hasOwnProperty(key)) {
      
      //var currentMesh = new THREE.Mesh( pmtGeometry, adMaterial );
      
      //currentMesh.scale.setY( .5 );
      
      //currentMesh.position.x = calibAd[key].centerX - detOrigin.x;
      
      //currentMesh.position.y = calibAd[key].centerY - detOrigin.y;
      
      //currentMesh.position.z = calibAd[key].centerZ - detOrigin.z;

      //scene.add( currentMesh );

    //}
  //}
  
  var loader = new THREE.JSONLoader();

  loader.load('js/PmtModel.js', function(geometry){
    
    /* calculate mesh color with linear gradient of rainbow */
    /* small charge to cold color and large charge to hot color */
    var first = true;
    var chgmin, chgmax;
    var keymin, keymax;
    
    for(var key in calibAd) {
      if(calibAd.hasOwnProperty(key)) {
        if(!calibAd[key].isHit) continue;

        /// find the smallest and largest charges
        if(first) {
          chgmin = calibAd[key].totCharge;
          keymin = key;
          chgmax = calibAd[key].totCharge;
          keymax = key;
          first = false;
        }
        else {
          if(calibAd[key].totCharge - chgmin < 0) {
            chgmin = calibAd[key].totCharge;
            keymin = key;
          }
          if(calibAd[key].totCharge - chgmax > 0) {
            chgmax = calibAd[key].totCharge;
            keymax = key;
          }
        }
      }
    }

    
    for(var key in calibAd) {
      if(calibAd.hasOwnProperty(key)) {
        
        var curColor = 0xCCCCFF;
        var frameType = true;
        if(calibAd[key].isHit) {
          curColor = calculateColorFromCharge(chgmin, chgmax, calibAd[key].totCharge);
          frameType = false;
        }
        
        adMaterial  = new THREE.MeshBasicMaterial( { color: curColor, wireframe: frameType } );
        
        var mesh = new THREE.Mesh( geometry, adMaterial );
        if(Math.abs(calibAd[key].normalZ) < 0.5)
          mesh.scale.set( 100, 100, 100 );
        else
          mesh.scale.set( 25, 25, 25 );
        
        mesh.position.x = calibAd[key].centerX - detOrigin.x;
        mesh.position.y = calibAd[key].centerY - detOrigin.y;
        mesh.position.z = calibAd[key].centerZ - detOrigin.z;
        
        /// rotate AD PMTs
        var axis = new THREE.Vector3();
        var dir0 = new THREE.Vector3(0,1,0);
        var ez = new THREE.Vector3(0,0,1);
        var dirDest = new THREE.Vector3(calibAd[key].normalX,calibAd[key].normalY,calibAd[key].normalZ)
        axis.crossVectors(dir0,dirDest);
        axis.normalize();
        
        var rotationAngle = Math.acos(dir0.dot(dirDest));
        if(axis.dot(ez) < 0) rotationAngle = -rotationAngle;
        
        if(Math.abs(calibAd[key].normalZ) < 0.5)
          mesh.rotation.set(0,0,rotationAngle);
        else if(calibAd[key].normalZ > 0.5)
          mesh.rotation.set(Math.PI/2,0,0);
        else if(calibAd[key].normalZ < -0.5)
          mesh.rotation.set(-Math.PI/2,0,0);
        
        mesh.matrix.setRotationFromEuler(mesh.rotation); // Set initial rotation
        
        scene.add( mesh );
      }
    }
  });
  
}



function getTrigger(triggerId) {
  $.ajax({
    async: false,
    url: 'php/queryTrigger.php',
    data: {"detectorTriggerId": triggerId},
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


function getTriggerId() {
  $.ajax({
    async: false,
    url: 'php/queryTriggerId.php',
    dataType: 'json',
    success: function(data) {
      $.each(data, function(i, row) {
        var obj = {
          detectorTriggerId: row.detectorTriggerId
	    };
	    validTrigId.push(obj);
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
  
  camera.position.z = 20000;
  
  scene = new THREE.Scene();
  
  var ambientLight = new THREE.AmbientLight(0x555555);
  scene.add(ambientLight);
  
  if(curTrig[0].detectorId <= 4) attachAd();
  
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



function processAdTrigger(triggerId) {

  /// clear container array
  calibAd = [];

  /// query AD origin
  var qrystr = 'SELECT originX, originY, originZ';
  qrystr += ' FROM detector';
  qrystr += ' WHERE detectorId=' + curTrig[0].detectorId;
  qrystr += ' AND hallId=' + curTrig[0].hallId;
  
  var adOrigin;
  
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
	    adOrigin = obj;
      })
    },
    error: function(error) {
      alert(error);
    }
  });
  
  detOrigin = adOrigin;


  /// query PMT geometry information
  qrystr = 'SELECT *';
  qrystr += ' FROM adPmt';
  qrystr += ' WHERE detectorId=' + curTrig[0].detectorId;
  qrystr += ' AND hallId=' + curTrig[0].hallId;

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
	    calibAd[row.adPmtId] = obj;
      calibAd[row.adPmtId].isHit = false;
      })
    },
    error: function(error) {
      alert(error);
    }
  });

  /// query hit PMT data
  qrystr = 'SELECT adPmtId, totCharge';
  qrystr += ' FROM calibAdPmt';
  qrystr += ' WHERE detectorTriggerId=' + triggerId;

  $.ajax({
    async: false,
    type: 'POST',
    url: 'php/queryDatabase.php',
    data: {qrystr: qrystr},
    dataType: 'json',
    success: function(data) {
      $.each(data, function(i, row) {
        calibAd[row.adPmtId].isHit = true;
        calibAd[row.adPmtId].totCharge = row.totCharge;
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


function processRpcTrigger() {
  $("#container").html("RPC trigger");
}


function processWsTrigger() {
  $("#container").html("WS trigger");
}



function render() {
  renderer.render( scene, camera );
}

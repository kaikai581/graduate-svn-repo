// retrieve PMT center locations from the database 

var adPmt    = new Array();
var wpPmt    = new Array();
var rpcStrip = new Array();

// variables for WebGL rendering
var container;
var camera, scene, renderer, controls;
var pmtGeometry, adMaterial;
var rpcStripGeometry, rpcMaterial;

var detectorElementMesh = new Array(7);
for(var i = 0; i < 7; i++) detectorElementMesh[i] = {};


$(document).ready(function() {
  // getJSON is ASYNCHRONOUS. Therefore three.js will get no data!
  // One has to resort to $.ajax() with async set to false.

  $(".detector").prop("checked", true);
  $(".detector").prop('disabled', false);
  // set default hall
  $("#EH").val(4);
  //$('#EH option').eq(2).prop("selected", true);

  // get AD PMT data
  getAdPmtData();
  //$.ajax({
    //async: false,
    //url: 'php/queryAdPmtPosition.php',
    //data: {"hall": $("#EH").val()},
    //dataType: "json",
    //success: function(data) {
      //$.each(data, function(i, row) {
        ////adPmtx.push(row.x_site);
        ////adPmty.push(row.y_site);
        ////adPmtz.push(row.z_site);
        //var obj = {
          //detector: row.detector,
          //x_site: row.x_site,
          //y_site: row.y_site,
          //z_site: row.z_site
	    //};
	    //adPmt.push(obj);
      //})
    //},
    //error: function(error) {
      //alert(error);
    //}
  //});
  
  
  // get WP PMT data
  getWpPmtData();
  getRpcStripData();
  
  // render the scene
  init();
  animate();
  //render();
  /*
  $("#check1").click(function(){
    if($("#check1").attr("checked")){
      for(var i = 0; i < 198; i++) detectorElementMesh[i].visible = true;
      render();
    }
    else{
      for(var i = 0; i < 198; i++) detectorElementMesh[i].visible = false;
      render();
    }
  });
  */
  
  $(".detector").click(function(){
    var det = $(this).val() - 1;

    for(var pmtId in detectorElementMesh[det]){
      if (detectorElementMesh[det].hasOwnProperty(pmtId)) {
        var currentVisibility = detectorElementMesh[det][pmtId].visible;
        detectorElementMesh[det][pmtId].visible = !currentVisibility;
      }
    }
    render();
  });
  //$("#check7").hide();
  //$("#RPC").hide();
  $("#EH").change(function(){
    if($(this).val() != 4){
      $("#check3").prop('disabled', true);
      $("#check4").prop('disabled', true);
    }
    else{
      $("#check3").prop('disabled', false);
      $("#check4").prop('disabled', false);
    }
    
    // select all detector
    $(".detector").prop("checked", true);
    
    // clear all objects in the scene including meshes
    clearScene();
    getAdPmtData();
    getWpPmtData();
    getRpcStripData();
    attachAd( pmtGeometry, adMaterial );
    attachWp( pmtGeometry, wpMaterial );
    attachRpc( rpcStripGeometry, rpcMaterial );
    render();
  });
  
});
  
// end of retrieving PMT center locations


function init() {

  camera = new THREE.PerspectiveCamera( 25, window.innerWidth / window.innerHeight, 1, 100000 );
  //camera = new THREE.PerspectiveCamera( 25, 800 / 480, 1, 100000 );
  camera.position.z = 20000;
  


  scene = new THREE.Scene();

  pmtGeometry      = new THREE.SphereGeometry( 100, 16, 16 );
  rpcStripGeometry = new THREE.CubeGeometry( 2080, 260, 2 );
  adMaterial  = new THREE.MeshBasicMaterial( { color: 0xFFFF66, wireframe: true } );
  wpMaterial  = new THREE.MeshBasicMaterial( { color: 0xADFF2F, wireframe: true } );
  rpcMaterial = new THREE.MeshBasicMaterial( { color: 0x0055ff, wireframe: true } );
  //adMaterial = new THREE.MeshLambertMaterial( { color: 0xFFFF66 } );
  //adMaterial = new THREE.MeshBasicMaterial( { color: 0xADFF2F } );
  //adMaterial = new THREE.MeshBasicMaterial( { color: 0xff0000, wireframe: true } );
  
  
  attachAd(pmtGeometry, adMaterial);
  //for (var pmtId = 0; pmtId < adPmt.length; pmtId++) {
    
    //var currentMesh = new THREE.Mesh( pmtGeometry, adMaterial );
    ////detectorElementMesh[pmtId].scale.setY( .5 );
    
    //currentMesh.position.x = adPmt[pmtId].x_site;
    //currentMesh.position.y = adPmt[pmtId].y_site;
    //currentMesh.position.z = adPmt[pmtId].z_site;
    //detectorElementMesh[adPmt[pmtId].detector-1].push(currentMesh);
    //scene.add( currentMesh );
    
  //}
  
  attachWp( pmtGeometry, wpMaterial );
  //for (var pmtId = 0; pmtId < wpPmt.length; pmtId++) {
    
    //var currentMesh = new THREE.Mesh( pmtGeometry, wpMaterial );

    //currentMesh.position.x = wpPmt[pmtId].x_site;
    //currentMesh.position.y = wpPmt[pmtId].y_site;
    //currentMesh.position.z = wpPmt[pmtId].z_site;
    //detectorElementMesh[wpPmt[pmtId].detector-1].push(currentMesh);
    //scene.add( currentMesh );
    
  //}
  attachRpc( rpcStripGeometry, rpcMaterial );

  //renderer = new THREE.CanvasRenderer();
  renderer = new THREE.WebGLRenderer();
  //renderer.setSize( 800, 480 );
  renderer.setSize( window.innerWidth, window.innerHeight );
  
  //container = document.getElementById( 'container' );
  container = document.getElementById( 'arena' );
  //container.appendChild( $("#widgets") );
  container.appendChild( renderer.domElement );
  //container.append( renderer.domElement );
  
  //renderer.domElement.addEventListener( 'drag', render, false );
  //renderer.domElement.addEventListener( 'mouseup', render, false );
  //renderer.domElement.addEventListener( 'mousemove', render, false );
  //window.addEventListener( 'mousedown', render );
  
  
  
  controls = new THREE.TrackballControls( camera, renderer.domElement );
  //controls = new THREE.TrackballControls( camera, document.getElementById("container") );
  //controls.target.set( 0, 0, 0 );

  //controls = new THREE.TrackballControls( camera );
  controls.rotateSpeed = 1.0;
  controls.zoomSpeed = 1.2;
  controls.panSpeed = 0.8;
  controls.noZoom = false;
  controls.noPan = false;
  controls.staticMoving = true;
  controls.dynamicDampingFactor = 0.3;
  controls.keys = [ 65, 83, 68 ];
  controls.addEventListener( 'change', render );
  
  // create a point light
  var pointLight = new THREE.PointLight(0xFFFFFF);
  
  // set its position
  pointLight.position.x = 10;
  pointLight.position.y = 10;
  pointLight.position.z = 130;
  
  // add to the scene
  scene.add(pointLight);

  //document.body.appendChild( renderer.domElement );
  window.addEventListener( 'resize', onWindowResize, false );

}

function onWindowResize() {
  
  camera.aspect = window.innerWidth / window.innerHeight;
  camera.updateProjectionMatrix();
  
  renderer.setSize( window.innerWidth, window.innerHeight );
  
  controls.handleResize();
  
  render();
  
}

function animate() {

  // note: three.js includes requestAnimationFrame shim
  requestAnimationFrame( animate );

  //detectorElementMesh.rotation.x += 0.01;
  //detectorElementMesh.rotation.y += 0.02;

  //renderer.render( scene, camera );
  controls.update();
  //render();
}


function render() {
  //alert("hi");
  //controls.update();
  //renderer.clear();
  renderer.render( scene, camera );
}


function clearScene() {
  for(var i = 0; i < 7; i++) {
    for(var j in detectorElementMesh[i]) {
      if( detectorElementMesh[i].hasOwnProperty(j) ) {
        scene.remove( detectorElementMesh[i][j] );
      }
    }
    detectorElementMesh[i] = {};
  }
}


function degree2radian(degree) {
  return degree*(Math.PI/180);
}


function getAdPmtData() {
  // empty the container array
  adPmt.length = 0;
  
  $.ajax({
    async: false,
    url: 'php/queryAdPmtPosition.php',
    data: {"hall": $("#EH").val()},
    dataType: "json",
    success: function(data) {
      $.each(data, function(i, row) {
        var obj = {
          detector: row.detector,
          ring    : row.ring,
          col     : row.column,
          x_site  : row.x_site,
          y_site  : row.y_site,
          z_site  : row.z_site
	    };
	    adPmt.push(obj);
      })
    },
    error: function(error) {
      alert(error);
    }
  });
}


function getRpcStripData() {
  // empty the container array
  rpcStrip.length = 0;
  
  $.ajax({
    async: false,
    url: 'php/queryRpcStripPosition.php',
    data: {"hall": $("#EH").val()},
    dataType: "json",
    success: function(data) {
      $.each(data, function(i, resrow) {
        var obj = {
          site    : resrow.site,
          detector: resrow.detector,
          row     : resrow.row,
          col     : resrow.col,
          layer   : resrow.layer,
          strip   : resrow.strip,
          x_site  : resrow.x_shifted_global,
          y_site  : resrow.y_shifted_global,
          z_site  : resrow.z_shifted_global,
          nx      : resrow.normal_xglobal,
          ny      : resrow.normal_yglobal,
          nz      : resrow.normal_zglobal
      };
      rpcStrip.push(obj);
      })
    },
    error: function(error) {
      alert(error);
    }
  });
}


function getWpPmtData() {
  // empty the container array
  wpPmt.length = 0;
  
  $.ajax({
    async: false,
    url: 'php/queryWpPmtPosition.php',
    data: {"hall": $("#EH").val()},
    dataType: "json",
    success: function(data) {
      $.each(data, function(i, row) {
        var obj = {
          detector      : row.detector,
          wall_number   : row.wall_number,
          wall_spot     : row.wall_spot,
          inward_facing : row.inward_facing,
          x_site        : row.x_site,
          y_site        : row.y_site,
          z_site        : row.z_site
	    };
	    wpPmt.push(obj);
      })
    }
  });
}


function attachRpc( rpcStripGeometry, rpcMaterial ) {
  
  /// local to global rotation angle
  var l2gRotAngle = {
    1: 2.145009114803047,
    2: -1.3912024468683883,
    4: -2.6275930112560291
  }
  
  for (var stripId = 0; stripId < rpcStrip.length; stripId++) {
    
    var currentMesh = new THREE.Mesh( rpcStripGeometry, rpcMaterial );

    var rotAngle;
    if(rpcStrip[stripId].layer == 1 || rpcStrip[stripId].layer == 4)
      rotAngle = degree2radian(90.)+l2gRotAngle[rpcStrip[stripId].site]; //correct one
    else
      rotAngle = l2gRotAngle[rpcStrip[stripId].site];
    
    var curPos = new THREE.Vector3(rpcStrip[stripId].x_site,rpcStrip[stripId].y_site,rpcStrip[stripId].z_site);
    var up = new THREE.Vector3(0, 0, 1);
    var normal = new THREE.Vector3(rpcStrip[stripId].nx, rpcStrip[stripId].ny, rpcStrip[stripId].nz);
    var tiltAngle = Math.acos(up.dot(normal));
    var axis = new THREE.Vector3();
    axis.cross(up, normal);
    axis.normalize();

    var tilt = new THREE.Matrix4().makeRotationAxis(axis, tiltAngle);
    var rotation = new THREE.Matrix4().makeRotationAxis(up, rotAngle);
    var totalRotation = tilt.multiplySelf(rotation);
    
    currentMesh.matrix = totalRotation;
    currentMesh.matrix.setPosition(curPos);
    currentMesh.matrixAutoUpdate = false;
    //currentMesh.updateMatrix();
    
    var id = [rpcStrip[stripId].row,rpcStrip[stripId].col,rpcStrip[stripId].layer,rpcStrip[stripId].strip];
    detectorElementMesh[rpcStrip[stripId].detector-1][id] = currentMesh;
    scene.add( currentMesh );
    
  }
}


function attachAd( pmtGeometry, adMaterial ) {
  for (var pmtId = 0; pmtId < adPmt.length; pmtId++) {
    
    var currentMesh = new THREE.Mesh( pmtGeometry, adMaterial );
    //detectorElementMesh[pmtId].scale.setY( .5 );
    
    currentMesh.position.x = adPmt[pmtId].x_site;
    currentMesh.position.y = adPmt[pmtId].y_site;
    currentMesh.position.z = adPmt[pmtId].z_site;
    
    var id = [adPmt[pmtId].ring, adPmt[pmtId].col];
    detectorElementMesh[adPmt[pmtId].detector-1][id] = currentMesh;
    scene.add( currentMesh );
    
  }
}


function attachWp( pmtGeometry, wpMaterial ) {
  for (var pmtId = 0; pmtId < wpPmt.length; pmtId++) {
    
    var currentMesh = new THREE.Mesh( pmtGeometry, wpMaterial );

    currentMesh.position.x = wpPmt[pmtId].x_site;
    currentMesh.position.y = wpPmt[pmtId].y_site;
    currentMesh.position.z = wpPmt[pmtId].z_site;
    
    var id = [wpPmt[pmtId].wall_number, wpPmt[pmtId].wall_spot, wpPmt[pmtId].inward_facing];
    detectorElementMesh[wpPmt[pmtId].detector-1][id] = currentMesh;
    scene.add( currentMesh );
    
  }
}

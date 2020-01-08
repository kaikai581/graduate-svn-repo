// retrieve PMT center locations from the database 

//var adPmtx = new Array();
//var adPmty = new Array();
//var adPmtz = new Array();
var adPmt = new Array();

//var wpPmtx = new Array();
//var wpPmty = new Array();
//var wpPmtz = new Array();
var wpPmt = new Array();

// variables for WebGL rendering
var container;
var camera, scene, renderer, controls;
var geometry, adMaterial;
var detectorElementMesh = new Array(7);

for(var i = 0; i < 7; i++) detectorElementMesh[i] = new Array();


$(document).ready(function() {
  // getJSON is ASYNCHRONOUS. Therefore three.js will get no data!
  // One has to resort to $.ajax() with async set to false.
  
  //$.getJSON('php/queryAdPmtPosition.php', processAdData);
  //function processAdData(data) {
    //var infoHTML = '';
    //$.each(data, function(i, row) {
      //adPmtx.push(row.x_site);
      //adPmty.push(row.y_site);
      //adPmtz.push(row.z_site);
    //})
  //}
  //$("#check1").prop("checked", true);
  //$("#check2").prop("checked", true);
  //$("#check3").prop("checked", true);
  //$("#check4").prop("checked", true);
  //$("#check5").prop("checked", true);
  //$("#check6").prop("checked", true);
  //$("#check7").prop("checked", true);
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
    var npmt = detectorElementMesh[det].length;
    for(var i = 0; i < npmt; i++){
      var currentVisibility = detectorElementMesh[det][i].visible;
      detectorElementMesh[det][i].visible = !currentVisibility;
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
    attachAd( geometry, adMaterial );
    attachWp( geometry, wpMaterial );
    render();
  });
  
});
  
// end of retrieving PMT center locations


function init() {

  camera = new THREE.PerspectiveCamera( 25, window.innerWidth / window.innerHeight, 1, 100000 );
  //camera = new THREE.PerspectiveCamera( 25, 800 / 480, 1, 100000 );
  camera.position.z = 20000;
  


  scene = new THREE.Scene();

  geometry = new THREE.SphereGeometry( 100, 16, 16 );
  adMaterial = new THREE.MeshLambertMaterial( { color: 0xFFFF66 } );
  //adMaterial = new THREE.MeshBasicMaterial( { color: 0xADFF2F } );
  //adMaterial = new THREE.MeshBasicMaterial( { color: 0xff0000, wireframe: true } );
  
  
  attachAd(geometry, adMaterial);
  //for (var pmtId = 0; pmtId < adPmt.length; pmtId++) {
    
    //var currentMesh = new THREE.Mesh( geometry, adMaterial );
    ////detectorElementMesh[pmtId].scale.setY( .5 );
    
    //currentMesh.position.x = adPmt[pmtId].x_site;
    //currentMesh.position.y = adPmt[pmtId].y_site;
    //currentMesh.position.z = adPmt[pmtId].z_site;
    //detectorElementMesh[adPmt[pmtId].detector-1].push(currentMesh);
    //scene.add( currentMesh );
    
  //}


  wpMaterial = new THREE.MeshBasicMaterial( { color: 0xADFF2F, wireframe: true } );
  
  attachWp( geometry, wpMaterial );
  //for (var pmtId = 0; pmtId < wpPmt.length; pmtId++) {
    
    //var currentMesh = new THREE.Mesh( geometry, wpMaterial );

    //currentMesh.position.x = wpPmt[pmtId].x_site;
    //currentMesh.position.y = wpPmt[pmtId].y_site;
    //currentMesh.position.z = wpPmt[pmtId].z_site;
    //detectorElementMesh[wpPmt[pmtId].detector-1].push(currentMesh);
    //scene.add( currentMesh );
    
  //}


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
    var nelement = detectorElementMesh[i].length;
    for(var j = 0; j < nelement; j++) {
      scene.remove( detectorElementMesh[i][j] );
    }
    detectorElementMesh[i].length = 0;
  }
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
          x_site: row.x_site,
          y_site: row.y_site,
          z_site: row.z_site
	    };
	    adPmt.push(obj);
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
          detector: row.detector,
          x_site: row.x_site,
          y_site: row.y_site,
          z_site: row.z_site
	    };
	    wpPmt.push(obj);
      })
    }
  });
}


function attachAd( geometry, adMaterial ) {
  for (var pmtId = 0; pmtId < adPmt.length; pmtId++) {
    
    var currentMesh = new THREE.Mesh( geometry, adMaterial );
    //detectorElementMesh[pmtId].scale.setY( .5 );
    
    currentMesh.position.x = adPmt[pmtId].x_site;
    currentMesh.position.y = adPmt[pmtId].y_site;
    currentMesh.position.z = adPmt[pmtId].z_site;
    detectorElementMesh[adPmt[pmtId].detector-1].push(currentMesh);
    scene.add( currentMesh );
    
  }
}


function attachWp( geometry, wpMaterial ) {
  for (var pmtId = 0; pmtId < wpPmt.length; pmtId++) {
    
    var currentMesh = new THREE.Mesh( geometry, wpMaterial );

    currentMesh.position.x = wpPmt[pmtId].x_site;
    currentMesh.position.y = wpPmt[pmtId].y_site;
    currentMesh.position.z = wpPmt[pmtId].z_site;
    detectorElementMesh[wpPmt[pmtId].detector-1].push(currentMesh);
    scene.add( currentMesh );
    
  }
}

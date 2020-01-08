// retrieve PMT center locations from the database 

var centerx = new Array();
var centery = new Array();
var centerz = new Array();
// variables for WebGL rendering
var camera, scene, renderer, controls;
var geometry, material;
var mesh = new Array();

$(document).ready(function() {
  $.getJSON('MySQLComm.php', processData);
  function processData(data) {
    var infoHTML = '';
    $.each(data, function(i, row) {
      centerx.push(row.x_site);
      centery.push(row.y_site);
      centerz.push(row.z_site);
      //infoHTML+='<p>' + row.x_site + '</p>';
    })
    /*
    for (var k = 0; k < centerx.length; k++) {
      infoHTML+='<p>'+centerx[k]+' '+centery[k]+' '+centerz[k]+'</p>';
    }
    $("div").html(infoHTML)
    */
    
    // render the scene
    init();
    //animate();
    render();
  }
});
  
// end of retrieving PMT center locations


function init() {

  camera = new THREE.PerspectiveCamera( 25, window.innerWidth / window.innerHeight, 1, 100000 );
  camera.position.z = 20000;


  scene = new THREE.Scene();

  //geometry = new THREE.CubeGeometry( 200, 200, 200 );
  geometry = new THREE.SphereGeometry( 100, 16, 16 );
  material = new THREE.MeshLambertMaterial( { color: 0xFFFF66 } );
  //material = new THREE.MeshBasicMaterial( { color: 0xFFFF66 } );
  //material = new THREE.MeshBasicMaterial( { color: 0xff0000, wireframe: true } );
  
  for (var pmtIdx = 0; pmtIdx < centerx.length; pmtIdx++) {
    
    mesh[pmtIdx] = new THREE.Mesh( geometry, material );
    //mesh[pmtIdx].scale.setY( .5 );
    mesh[pmtIdx].translate(centerx[pmtIdx], new THREE.Vector3(1,0,0));
    mesh[pmtIdx].translate(centery[pmtIdx], new THREE.Vector3(0,1,0));
    mesh[pmtIdx].translate(centerz[pmtIdx], new THREE.Vector3(0,0,1));
    scene.add( mesh[pmtIdx] );
    
  }

  //renderer = new THREE.CanvasRenderer();
  renderer = new THREE.WebGLRenderer();
  renderer.setSize( window.innerWidth, window.innerHeight );
  
  controls = new THREE.TrackballControls( camera, renderer.domElement );
  controls.target.set( 0, 0, 0 );
  controls.rotateSpeed = 1.0;
  controls.staticMoving = true;
  controls.keys = [ 65, 83, 68 ];
  //renderer.domElement.addEventListener( 'drag', render, false );
  //renderer.domElement.addEventListener( 'mouseup', render, false );
  renderer.domElement.addEventListener( 'mousemove', render, false );
  //window.addEventListener( 'mousedown', render );
  
  // create a point light
  var pointLight = new THREE.PointLight(0xFFFFFF);
  
  // set its position
  pointLight.position.x = 10;
  pointLight.position.y = 10;
  pointLight.position.z = 130;
  
  // add to the scene
  scene.add(pointLight);

  document.body.appendChild( renderer.domElement );

}

function animate() {

  // note: three.js includes requestAnimationFrame shim
  requestAnimationFrame( animate );

  //mesh.rotation.x += 0.01;
  //mesh.rotation.y += 0.02;

  //renderer.render( scene, camera );

}


function render() {
  //alert("hi");
  controls.update();
  //renderer.clear();
  renderer.render( scene, camera );
}

<?php
$con = mysql_connect("localhost","sklin","aaaa5816");
if (!$con)
{
  die('Could not connect: ' . mysql_error());
}

mysql_select_db("MuonDB", $con);

$qrystr = "SELECT * FROM rpc_strip_locations WHERE site=".json_decode($_GET["hall"]);
$result = mysql_query($qrystr);

$rows = array();

while($row = mysql_fetch_array($result))
{
  $rows[] = $row;
}
echo json_encode($rows);

mysql_close($con);

?>

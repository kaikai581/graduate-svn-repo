<?php
$con = mysql_connect("localhost","sklin","aaaa5816");
if (!$con)
{
  die('Could not connect: ' . mysql_error());
}

mysql_select_db("MuonDB", $con);

$result = mysql_query("SELECT * FROM wp_pmt_locations WHERE site=".json_decode($_GET["hall"]));

$rows = array();

while($row = mysql_fetch_array($result))
{
  $rows[] = $row;
}
echo json_encode($rows);

mysql_close($con);

?>

<?php

$con = mysql_connect("localhost","sklin","aaaa5816");
if (!$con)
{
  die('Could not connect: ' . mysql_error());
}

mysql_select_db("EventDisplayDB_v042", $con);

$qrystr = "SELECT adPmtId, totCharge FROM calibAdPmt WHERE detectorTriggerId=".json_decode($_GET["triggerId"]);

$result = mysql_query($qrystr);

$rows = array();

while($row = mysql_fetch_array($result))
{
  $rows[] = $row;
}
echo json_encode($rows);

mysql_close($con);

?>

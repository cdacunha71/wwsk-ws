<?php
include 'vars.php';
header("Access-Control-Allow-Origin: *");


// Create connection
$conn = new mysqli($connectstr_dbhost, $connectstr_dbusername, $connectstr_dbpassword,$connectstr_dbname);
// Check connection
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
} 

$sql = "SELECT date_format(TIME,'%Y-%m-%dT%H:%i:%s') AS TIME,ROUND(AVG(PRESSURE)) AS PRESSURE FROM `WSDATA` WHERE TIME > DATE_SUB(NOW(), INTERVAL 24 HOUR) GROUP BY UNIX_TIMESTAMP(TIME) DIV 300";
$result = $conn->query($sql);
$measurements = array();
while($returned_array = $result->fetch_assoc()){
	array_push($measurements,$returned_array);
}
//$measurements = array_reverse($measurements); SQL query orders correctly
$conn->close();


  //$rows = array();
  $table = array();
  $table['cols'] = array(

    // Labels for your chart, these represent the column titles.
    /* 
        note that one column is in "string" format and another one is in "number" format 
        as pie chart only required "numbers" for calculating percentage 
        and string will be used for Slice title
    */

    array('label' => 'Time', 'type' => 'datetime'),
    array('label' => 'Pressure', 'type' => 'number')
	);
    /* Extract the information from $result */
		
    foreach($measurements as $r) { 
			  
	  $table['rows'][]['c'] = array(

		array('v' => (string) $r['TIME'] ),
		array('v' => (string) $r['PRESSURE'])
        );
	}
	//convert data into JSON format
	$jsonTable = json_encode($table,JSON_NUMERIC_CHECK);
	echo $jsonTable;
?>

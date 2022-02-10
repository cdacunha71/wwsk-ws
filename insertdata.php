<?php
include 'vars.php';

function convertSpeed($speed){
	$speed = $speed + 0;
     switch(true)
    {
    case ($speed > 64):
    $speedbf = "12";
    break;
	case ($speed > 56):
    $speedbf = "11";
    break;
	case ($speed > 48):
    $speedbf = "10";
    break;
	case ($speed > 41):
    $speedbf = "9";
    break;
	case ($speed > 34):
    $speedbf = "8";
    break;
	case ($speed > 28):
    $speedbf = "7";
    break;
	case ($speed > 22):
    $speedbf = "6";
    break;
	case ($speed > 17):
    $speedbf = "5";
    break;
	case ($speed > 11):
    $speedbf = "4";
    break;
	case ($speed > 7):
    $speedbf = "3";
    break;
	case ($speed > 4):
    $speedbf = "2";
    break;
	case ($speed > 1):
    $speedbf = "1";
    break;
	default:
	$speedbf = "0";
	}

 return $speedbf;
}

// S,G,D,T,P
parse_str($_SERVER['QUERY_STRING'],$params);

$S = $params['S'];
$G = $params['G'];
$D = $params['D'];
$T = $params['T'];
$P = $params['P'];

// Create connection
$conn = new mysqli($connectstr_dbhost, $connectstr_dbusername, $connectstr_dbpassword,$connectstr_dbname);
// Check connection
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
} 

$SB = convertSpeed($S);
$GB = convertSpeed($G);

$sql = "INSERT INTO WSDATA (SPEED,SPEEDB,GUST,GUSTB,DIRECTION,TEMPERATURE,PRESSURE)
VALUES ($S, $SB, $G, $GB, $D, $T, $P)";

if ($conn->query($sql) === TRUE) {
    echo "OK";
} else {
    echo "Error: " . $sql . "<br>" . $conn->error;
}

$conn->close();
?>

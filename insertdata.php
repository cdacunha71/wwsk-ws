<?php
include 'vars.php';


// S,D,T
parse_str($_SERVER['QUERY_STRING']);

// Create connection
$conn = new mysqli($connectstr_dbhost, $connectstr_dbusername, $connectstr_dbpassword,$connectstr_dbname);
// Check connection
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
} 

$sql = "INSERT INTO WSDATA (SPEED,DIRECTION,TEMPERATURE)
VALUES ($S, $D, $T)";

if ($conn->query($sql) === TRUE) {
    echo "New record created successfully";
} else {
    echo "Error: " . $sql . "<br>" . $conn->error;
}

$conn->close();
?>

<?php
$upload_dir = getenv("UPLOAD_DIR");

// Check if UPLOAD_DIR is defined and valid
if (!$upload_dir) {
    echo "Content-Type: text/html\r\n\r\n";
    echo "<h2>UPLOAD_DIR environment variable not set.</h2>";
    exit;
}
/* while (true)
{
	sleep(1);
} */

// Make sure directory exists
if (!is_dir($upload_dir)) {
    mkdir($upload_dir, 0777, true);
}
// Handle file upload
if ($_SERVER["REQUEST_METHOD"] === "POST" && isset($_FILES["file"])) {
    $filename = basename($_FILES["file"]["name"]);
    $ext = pathinfo($filename, PATHINFO_EXTENSION);
	if ($ext !== "txt" && $ext !== "jpg")
    {
        http_response_code(415);
        exit;
    }
	$filename_noext = pathinfo($filename, PATHINFO_FILENAME);
	if (!preg_match('/^[a-zA-Z0)9_-]+$/', $filename_noext))
	{
		http_response_code(400);
		exit;
	}
    $target_path = rtrim($upload_dir, '/') . '/' . $filename;

    if (move_uploaded_file($_FILES["file"]["tmp_name"], $target_path)) {
        echo "Content-Type: text/html\r\n\r\n";
        echo "<h2>File uploaded successfully to: $target_path</h2>";
    } else {
       http_response_code(403);
       exit ;
    }
} else {
    echo "Content-Type: text/html\r\n\r\n";
    echo "<h2>No file uploaded.</h2>";
}
?>

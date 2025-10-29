<?php
$upload_dir = getenv("UPLOAD_DIR");
$rel_upload_dir = getenv("RELATIVE_UPLOAD_DIR");

if (!$upload_dir || !is_dir($upload_dir) || !is_readable($upload_dir)) {
  http_response_code(403);  // Set HTTP status code to 403
  exit;
}

$files = array_diff(scandir($upload_dir), ['.', '..']);

echo "<ul>";
foreach ($files as $file) {
    $urlFile = htmlspecialchars($file);
    echo "<li>$urlFile 
            <button onclick=\"deleteFile('$urlFile')\">Delete</button>
          </li>";
}
echo "</ul>";
?>

<script>
var uploadDir = "<?php echo addslashes($rel_upload_dir); ?>";

function deleteFile(filename) {
  if (!confirm(`Delete ${filename}?`)) return;

  var deleteUrl = uploadDir + encodeURIComponent(filename);
  var ext = deleteUrl.split('.').pop()

  fetch(deleteUrl, {
    method: 'DELETE',
    headers: {
    'Content-Type': 'application/json',
    }
  })
  .then(res => res.text().then(html => {
    if (!res.ok) {
      document.open();
      document.write(html);
      document.close();
      return;
    }

    alert(html);
    location.reload();
  }))
  .catch(err => {
    console.error("Delete failed:", err);
  });
}
</script>

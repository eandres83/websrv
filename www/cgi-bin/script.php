<?php
// Utilidades
function h($v) { return htmlspecialchars((string)$v, ENT_QUOTES, 'UTF-8'); }

// Polyfill de getallheaders() para CGI/FPM
if (!function_exists('getallheaders')) {
    function getallheaders() {
        $headers = [];
        foreach ($_SERVER as $k => $v) {
            if (strpos($k, 'HTTP_') === 0) {
                $name = str_replace(' ', '-', ucwords(strtolower(str_replace('_', ' ', substr($k, 5)))));
                $headers[$name] = $v;
            } elseif (in_array($k, ['CONTENT_TYPE','CONTENT_LENGTH'])) {
                $name = str_replace('_', '-', ucwords(strtolower($k)));
                $headers[$name] = $v;
            }
        }
        return $headers;
    }
}

$method = $_SERVER['REQUEST_METHOD'] ?? 'GET';
$script = $_SERVER['SCRIPT_NAME'] ?? '/cgi-bin/script.php';
$qs     = $_SERVER['QUERY_STRING'] ?? '';
$uri    = $_SERVER['REQUEST_URI'] ?? ($script . ($qs ? "?$qs" : ''));
$server = $_SERVER['SERVER_NAME'] ?? 'localhost';
$port   = $_SERVER['SERVER_PORT'] ?? '8080';
$remote = $_SERVER['REMOTE_ADDR'] ?? 'unknown';
$proto  = $_SERVER['SERVER_PROTOCOL'] ?? 'HTTP/1.1';
$now    = date('Y-m-d H:i:s');

$headers = getallheaders();
$bodyRaw = file_get_contents('php://input'); // útil para JSON u otros tipos
?>
<!DOCTYPE html>
<html lang="es">
<head>
  <meta charset="utf-8">
  <title>CGI Demo PHP</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body{font-family:system-ui,-apple-system,Segoe UI,Roboto,Ubuntu,sans-serif;margin:2rem;line-height:1.4}
    header{margin-bottom:1rem}
    code,pre{background:#f6f8fa;border:1px solid #e5e7eb;border-radius:6px;padding:.25rem .5rem}
    table{border-collapse:collapse;width:100%;margin:.5rem 0}
    th,td{border:1px solid #e5e7eb;padding:.4rem .6rem;text-align:left;vertical-align:top}
    th{background:#fafafa}
    .grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(280px,1fr));gap:1rem}
    .card{border:1px solid #e5e7eb;border-radius:8px;padding:1rem}
    .ok{color:#0a7}
  </style>
</head>
<body>
<header>
  <h1>CGI PHP en funcionamiento</h1>
  <p class="ok">Servidor operativo ✅</p>
</header>

<section class="grid">
  <div class="card">
    <h2>Resumen</h2>
    <table>
      <tr><th>Método</th><td><?php echo h($method); ?></td></tr>
      <tr><th>URI</th><td><?php echo h($uri); ?></td></tr>
      <tr><th>Query String</th><td><?php echo h($qs); ?></td></tr>
      <tr><th>Protocolo</th><td><?php echo h($proto); ?></td></tr>
      <tr><th>Host</th><td><?php echo h($server.':'.$port); ?></td></tr>
      <tr><th>Cliente</th><td><?php echo h($remote); ?></td></tr>
      <tr><th>Fecha</th><td><?php echo h($now); ?></td></tr>
    </table>
  </div>

  <div class="card">
    <h2>Prueba rápida</h2>
    <form method="get" action="">
      <label>GET name: <input name="name" value="<?php echo isset($_GET['name'])?h($_GET['name']):''; ?>"></label>
      <button type="submit">Enviar GET</button>
    </form>
    <form method="post" action="" style="margin-top:.5rem">
      <label>POST message: <input name="message" value=""></label>
      <button type="submit">Enviar POST</button>
    </form>
  </div>
</section>

<section class="card">
  <h2>Parámetros GET</h2>
  <?php if ($_GET): ?>
    <table>
      <tr><th>Clave</th><th>Valor</th></tr>
      <?php foreach ($_GET as $k => $v): ?>
        <tr><td><?php echo h($k); ?></td><td><?php echo h(is_array($v)?json_encode($v):$v); ?></td></tr>
      <?php endforeach; ?>
    </table>
  <?php else: ?>
    <p>(sin parámetros)</p>
  <?php endif; ?>
</section>

<section class="card">
  <h2>Parámetros POST</h2>
  <?php if ($_POST): ?>
    <table>
      <tr><th>Clave</th><th>Valor</th></tr>
      <?php foreach ($_POST as $k => $v): ?>
        <tr><td><?php echo h($k); ?></td><td><?php echo h(is_array($v)?json_encode($v):$v); ?></td></tr>
      <?php endforeach; ?>
    </table>
  <?php else: ?>
    <p>(sin parámetros)</p>
  <?php endif; ?>

  <?php if ($bodyRaw && empty($_POST)): ?>
    <h3>Cuerpo bruto</h3>
    <pre><?php echo h($bodyRaw); ?></pre>
  <?php endif; ?>
</section>

<section class="card">
  <h2>Cabeceras</h2>
  <?php if ($headers): ?>
    <table>
      <tr><th>Header</th><th>Valor</th></tr>
      <?php foreach ($headers as $k => $v): ?>
        <tr><td><?php echo h($k); ?></td><td><?php echo h($v); ?></td></tr>
      <?php endforeach; ?>
    </table>
  <?php else: ?>
    <p>(sin cabeceras)</p>
  <?php endif; ?>
</section>

<section class="card">
  <h2>Variables de entorno</h2>
  <table>
    <tr><th>Clave</th><th>Valor</th></tr>
    <?php
      $keys = ['GATEWAY_INTERFACE','SERVER_PROTOCOL','SERVER_NAME','SERVER_PORT','REQUEST_METHOD','SCRIPT_FILENAME','PATH_INFO','QUERY_STRING'];
      foreach ($keys as $k) {
          $v = isset($_SERVER[$k]) ? $_SERVER[$k] : '';
          echo '<tr><td>'.h($k).'</td><td>'.h($v).'</td></tr>';
      }
    ?>
  </table>
</section>

<footer style="margin-top:1rem;color:#6b7280">
  <small>Generado por PHP (CGI)</small>
</footer>
</body>
</html>

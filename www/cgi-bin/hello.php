#!/usr/bin/php
<?php
// Script CGI PHP simples para teste GET
?>
Content-Type: text/html

<!DOCTYPE html>
<html lang='en'>
<head>
    <meta charset='UTF-8'>
    <title>PHP CGI - Hello</title>
    <style>
        body { font-family: Arial, sans-serif; max-width: 800px; margin: 50px auto; padding: 20px; }
        h1 { color: #787cb5; }
        .info { background: #f0f0f0; padding: 15px; border-radius: 5px; margin: 20px 0; }
        .success { color: green; font-weight: bold; }
    </style>
</head>
<body>
    <h1>🐘 PHP CGI Script</h1>
    <p class='success'>✅ CGI funcionando corretamente!</p>
    
    <div class='info'>
        <h2>Informações do PHP:</h2>
        <ul>
            <li><strong>PHP Version:</strong> <?php echo phpversion(); ?></li>
            <li><strong>REQUEST_METHOD:</strong> <?php echo getenv('REQUEST_METHOD'); ?></li>
            <li><strong>QUERY_STRING:</strong> <?php echo getenv('QUERY_STRING'); ?></li>
            <li><strong>SCRIPT_FILENAME:</strong> <?php echo getenv('SCRIPT_FILENAME'); ?></li>
            <li><strong>SERVER_PROTOCOL:</strong> <?php echo getenv('SERVER_PROTOCOL'); ?></li>
            <li><strong>GATEWAY_INTERFACE:</strong> <?php echo getenv('GATEWAY_INTERFACE'); ?></li>
        </ul>
    </div>
    
    <div class='info'>
        <h2>Dados da Requisição:</h2>
        <pre><?php print_r($_SERVER); ?></pre>
    </div>
    
    <p><a href='/'>← Voltar</a></p>
</body>
</html>


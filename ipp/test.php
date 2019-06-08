<?php

# Find file recursively in a directory
# Source: https://stackoverflow.com/questions/17160696/php-glob-scan-in-subfolders-for-a-file
function rglob($pattern, $flags = 0) {
    $files = glob($pattern, $flags); 
    foreach (glob(dirname($pattern).'/*', GLOB_ONLYDIR|GLOB_NOSORT) as $dir) {
        $files = array_merge($files, rglob($dir.'/'.basename($pattern), $flags));
    }
    return $files;
}

# Find all tests according to .src files
function get_files($directory=".", $recursive=false)
{
    if ($recursive)
    {
        $src_files = rglob($directory."/*.src");
    }
    else
    {
        $src_files = glob($directory."/*.src");
    }
    $files = [];
    foreach ($src_files as $file) # Loop through all tests and create testing array
    {
        $filename = pathinfo($file, PATHINFO_FILENAME);
        $directory = pathinfo($file, PATHINFO_DIRNAME);
        $files[$filename] = ["src" => $file];
        $in_file_name = $directory."/".$filename.".in";
        $in_file = glob($in_file_name);
        if (empty($in_file))
        {
            $in_file = touch($directory."/".$filename.".in");
        }
        $files[$filename]["in"] = $in_file_name;

        $out_file_name = $directory."/".$filename.".out";
        $out_file = glob($out_file_name);
        if (empty($out_file))
        {
            $out_file = touch($directory."/".$filename.".out");
        }
        $files[$filename]["out"] = $out_file_name;

        $rc_file_name = $directory."/".$filename.".rc";
        $rc_file = glob($rc_file_name);
        if (empty($rc_file))
        {
            $rc_file = file_put_contents($directory."/".$filename.".rc", "0");
        }
        $files[$filename]["rc"] = $rc_file_name;
    }
    return $files;

}

# Parse commandline argumens
$longopts = ['help', 'directory:', 'recursive', 'parse-script:', 'int-script:', 'parse-only', 'int-only'];
$args = getopt("", $longopts);
if (array_key_exists('help', $args))
{
    if (sizeof($args) > 1)
    {
        exit(10);
    }
    else
    {
        echo "This script runs tests of parse.php and interpret.py based on given arguments.\n Possible arguments are:\n--directory Sets directory with tests\n--recursive Sets to search the directory recursively
             \n--parse-script Path to the parse script\n--int-script Sets path to interpret.py\n--parse-pnly Test only the parser\n--int-only Test only the interpreter\n";
    }
}
if (array_key_exists('directory', $args))
{
    $directory = $args['directory'];
}
else
{
    $directory = ".";
}

if (array_key_exists('recursive', $args))
{
    $recursive = true;
}
else
{
    $recursive = false;
}
if (array_key_exists('parse-script', $args))
{
    $parse_script = $args['parse-script'];
}
else
{
    $parse_script = "./parse.php";
}
if (array_key_exists('int-script', $args))
{
    $int_script = $args['int-script'];
}
else
{
    $int_script = "./interpret.py";
}

# Prepare testing variables 
$conv_before = ['<', '>'];
$conv_after = ['&lt;', '&gt;'];
$testcases = get_files($directory, $recursive);
$test_output = [];
$test_rc = 0;
$diff_output = [];
$diff_rc = [];
$htmlinfo = "";
$htmlfile = "<!DOCTYPE html>
<html lang=\"en\">
<head>
    <meta charset=\"UTF-8\">
    <title>test.php report</title>
</head>
<body>
    <table border=\"1\">
        <tr>
            <th>Test</th>
            <th>Status</th>
        </tr>
";

if (array_key_exists('parse-only', $args) and array_key_exists('int-only', $args))
{
    exit(10);
}
elseif (array_key_exists('int-only', $args) and array_key_exists('parse-script', $args))
{   
    exit(10);
}
elseif (array_key_exists('parse-only', $args) and array_key_exists('int-script', $args))
{
    exit(10);
}
elseif (array_key_exists('parse-only', $args)) 
{
    foreach ($testcases as $test) # Perform tests on all testfiles in parse only
    {
        exec('php7.3 '.$parse_script.' <'.$test['src'].' >test.xml', $test_output, $test_rc);
        $temp_rc = file_get_contents($test['rc']);
        $htmlfile .= "<tr>
                        <td>".pathinfo($test['src'], PATHINFO_FILENAME)."</td>";
        if ((int)$temp_rc !== (int)$rc)
        {
            $htmlinfo .= "<p>Test: ".pathinfo($test['src'], PATHINFO_FILENAME)." failed: Invalid return code<br>Expected: ".$temp_rc."<br>Actual: ".$test_rc."</p>";
            $htmlfile .= "<td bgcolor=\"#FF0000\">FAIL</td>";
            #echo "fail";
        }
        elseif((int)$temp_rc === (int)$test_rc and $test_rc === 0)
        {
            #exec("java -jar jexam.jar test.xml ".$test['out'], $diff_output, $diff_rc); #home version
            exec("java -jar /pub/courses/ipp/jexamxml/jexamxml.jar test.xml ".$test['out'], $diff_output, $diff_rc); #merlin
            if ($diff_rc === 0)
            {
                $htmlfile .= "<td bgcolor=\"#00FF00\">OK</td>";
            }
            else
            {
                $htmlinfo .= "<p>Test: ".pathinfo($test['src'], PATHINFO_FILENAME)." failed: Invalid output<br>Expected:<br><pre lang=\"xml\">".str_replace($conv_before, $conv_after, file_get_contents($test['out']))."</pre><br>Actual:<br><pre lang=\"xml\">".str_replace($conv_before, $conv_after, file_get_contents('test.xml'))."</pre></p>";
                $htmlfile .= "<td bgcolor=\"#FF0000\">FAIL</td>";
            }
            #echo "success";
        }
        else
        {
            $htmlfile .= "<td bgcolor=\"#00FF00\">OK</td>";
        }
    }
}
elseif (array_key_exists('int-only', $args)) # Perform all int only tests
{
    foreach ($testcases as $test)
    {
        exec('python3 '.$int_script.' --source='.$test['src'].' --input='.$test['in'].' >test.out',$output,  $test_rc);
        #fwrite(STDERR, $test_rc);
        $temp_rc = file_get_contents($test['rc']);
        $htmlfile .= "<tr>
                        <td>".pathinfo($test['src'], PATHINFO_FILENAME)."</td>";
        if ((int)$temp_rc !== (int)$test_rc)
        {
            $htmlinfo .= "<p>Test: ".pathinfo($test['src'], PATHINFO_FILENAME)." failed: Invalid return code<br>Expected: ".$temp_rc."<br>Actual: ".$test_rc."</p>";
            $htmlfile .= "<td bgcolor=\"FF0000\">FAIL</td>";
        }
        elseif ((int)$temp_rc === (int)$test_rc and $test_rc === 0)
        {
            exec("diff test.out ".$test['out']." >diff.out", $diff_output, $diff_rc);
            if ($diff_rc === 0)
            {
                $htmlfile .= "<td bgcolor=\"#00FF00\">OK</td>";
            }
            else
            {
                $htmlinfo .= "<p>Test: ".pathinfo($test['src'], PATHINFO_FILENAME)." failed: Invalid output<br>Diff output:<br><pre>".file_get_contents("diff.out")."</pre></p>";
                $htmlfile .= "<td bgcolor=\"#FF0000\">FAIL</td>";
            }
        }
        else
        {
            $htmlfile .= "<td bgcolor=\"#00FF00\">OK</td>";
        }
    }
}
else # Perform test for both parse only and int only
{
    foreach($testcases as $test)
    {
        exec('php7.3 '.$parse_script.' <'.$test['src'].' >test.xml', $test_output, $test_rc);
        $temp_rc = file_get_contents($test['rc']);
        $htmlfile .= "<tr>
                        <td>".pathinfo($test['src'], PATHINFO_FILENAME)."</td>";
        if ($test_rc !== 0)
        {
            if ((int)$temp_rc !== (int)$test_rc)
            {
                $htmlinfo .= "<p>Test: ".pathinfo($test['src'], PATHINFO_FILENAME)." failed: Invalid return code<br>Expected: ".$temp_rc."<br>Actual: ".$test_rc."</p>";
                $htmlfile .= "<td bgcolor=\"FF0000\">FAIL</td>";
            }
            else
            {
                $htmlfile .= "<td bgcolor=\"#00FF00\">OK</td>";
            }
        }
        else
        {
            exec('python3 '.$int_script.' --source='.'test.xml'.' --input='.$test['in'].' >test.out',$output,  $test_rc);
            if ((int)$temp_rc !== (int)$test_rc)
            {
                $htmlinfo .= "<p>Test: ".pathinfo($test['src'], PATHINFO_FILENAME)." failed: Invalid return code<br>Expected: ".$temp_rc."<br>Actual: ".$test_rc."</p>";
                $htmlfile .= "<td bgcolor=\"FF0000\">FAIL</td>";
            }
            elseif ((int)$temp_rc === (int)$test_rc and $test_rc === 0)
            {
                exec("diff test.out ".$test['out']." >diff.out", $diff_output, $diff_rc);
                if ($diff_rc === 0)
                {
                    $htmlfile .= "<td bgcolor=\"#00FF00\">OK</td>";
                }
                else
                {
                    $htmlinfo .= "<p>Test: ".pathinfo($test['src'], PATHINFO_FILENAME)." failed: Invalid output<br>Diff output:<br><pre>".file_get_contents("diff.out")."</pre></p>";
                    $htmlfile .= "<td bgcolor=\"#FF0000\">FAIL</td>";
                }
            }
            else
            {
                $htmlfile .= "<td bgcolor=\"#00FF00\">OK</td>";
            }
        }
    }

}

# Print html to standard output
$htmlfile .= "</table>";
$htmlfile .= $htmlinfo;
$htmlfile .= "</body></html>";
echo $htmlfile;

?>

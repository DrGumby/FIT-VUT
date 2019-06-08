<?php

function print_help()
{
    echo "Program uses standard input to read IPPCode19 source file, performs lexical and syntactic checks and creates XML output\n";
}

function print_stats($STATP, $mask, $file)
{
    $openFile = fopen($file, "w");
    foreach($mask as $stat)
    {
        fwrite($openFile, $STATP[$stat]."\n");
    }

    fclose($openFile);
}

function parse_command_line_args($argv)
{
    if(count($argv) == 1)
    {
        return;
    }
    elseif (count($argv) == 2) 
    {
        if ($argv[1] == "--help")
        {
            print_help();
            exit(0);
        }
        else 
        {
            echo "ERROR: Invalid argument\n";
            exit(10);
        }
    }
    elseif (count($argv) > 1 and strpos($argv[1], '--stats=') === 0)         #argument starts with stats
    {
        $file = explode('=', $argv[1])[1];
        $mask = [];
        foreach(array_slice($argv, 2) as $key)
        {
            if(in_array($key, ["--loc", "--comments", "--jumps", "--labels"]))
            {
                array_push($mask, $key);
            }
            else 
            {
                echo "ERROR: Invalid argument\n";
                exit(10);
            }
        }
        return ["file" => $file,
                "mask" => $mask];
    }
    else
    {
        echo "Invalid number of arguments\n";
        exit(10);
    }
}

function parse_line($line, &$lineNumber, $xmlDoc, &$STATP)
{
    $line = trim($line);
    if($line === '')
    {
        return;
    }
    elseif($line[0] == '#')
    {
        $STATP["--comments"]++;
        return;
    }
    elseif(strlen(trim($line)) == 0)
    {
        return;
    }
    elseif(strpos($line, '#'))
    {
        $line = explode('#', $line)[0];
        $line = rtrim($line);
        $STATP["--comments"]++;
    }
    $STATP["--loc"]++;
    xmlwriter_start_element($xmlDoc, 'instruction');  #Generate instruction element
    $line_array = explode(" ", $line, 2);
    $expectedArgs = parse_opcode($line_array[0], $xmlDoc, $STATP, $lineNumber);
    parse_arguments($xmlDoc, $line_array, $expectedArgs);
    xmlwriter_end_element($xmlDoc);
}

function parse_string($inputString)
{
    $escapeArray = [];
    $escapeInt = 0;

    preg_match_all('/\\\\\d\d\d/', $inputString, $escapeArray);
    foreach($escapeArray[0] as $escapeSequence)
    {
        $escapeInt = intval(substr($escapeSequence, 1));
        if(in_array(chr($escapeInt), ['<', '>', '&']))
            $inputString = str_replace($escapeSequence, chr($escapeInt), $inputString, $i);
    }

    if(empty($escapeArray[0]) and preg_match('/\\\\/', $inputString) === 1)
    {
        exit(23);
    }

    
    return $inputString;
}

function parse_opcode($opcode, $xmlDoc, &$STATP, &$lineNumber)
{
    if($opcode[strlen($opcode)-1] == "\n")
    {
        $opcode = substr($opcode, 0, -1);
    }
    $arr_opcodes = ["MOVE"          => ["var", "symb"],
                    "CREATEFRAME"   => NULL,
                    "PUSHFRAME"     => NULL, 
                    "POPFRAME"      => NULL, 
                    "DEFVAR"        => ["var"],
                    "CALL"          => ["label"],
                    "RETURN"        => NULL,
                    "PUSHS"         => ["symb"],
                    "POPS"          => ["var"], 
                    "ADD"           => ["var", "symb", "symb"],
                    "SUB"           => ["var", "symb", "symb"],
                    "MUL"           => ["var", "symb", "symb"],
                    "IDIV"          => ["var", "symb", "symb"],
                    "LT"            => ["var", "symb", "symb"],
                    "GT"            => ["var", "symb", "symb"],
                    "EQ"            => ["var", "symb", "symb"],
                    "AND"           => ["var", "symb", "symb"],
                    "OR"            => ["var", "symb", "symb"],
                    "NOT"           => ["var", "symb"],
                    "INT2CHAR"      => ["var", "symb"],
                    "STRI2INT"      => ["var", "symb", "symb"], 
                    "READ"          => ["var", "type"],
                    "WRITE"         => ["symb"],
                    "CONCAT"        => ["var", "symb", "symb"],
                    "STRLEN"        => ["var", "symb"],
                    "GETCHAR"       => ["var", "symb", "symb"],
                    "SETCHAR"       => ["var", "symb", "symb"],
                    "TYPE"          => ["var", "symb"],
                    "LABEL"         => ["label"],
                    "JUMP"          => ["label"],
                    "JUMPIFEQ"      => ["label", "symb", "symb"],
                    "JUMPIFNEQ"     => ["label", "symb", "symb"], 
                    "EXIT"          => ["symb"],
                    "DPRINT"        => ["symb"],
                    "BREAK"         => NULL];
                    
    
    if(!array_key_exists(strtoupper($opcode), $arr_opcodes))
    {
       
        #echo("Invalid opcode: ".$opcode."\n");
        exit(22);        
    }
    else
    {
        if(in_array(strtoupper($opcode), ["JUMP", "JUMPIFEQ", "JUMPIFNEQ"]))
        {
            $STATP["--jumps"]++;
        }
        elseif(strtoupper($opcode) == "LABEL")
        {
            $STATP["--labels"]++;
        }
        xmlwriter_start_attribute($xmlDoc, 'order');
        xmlwriter_text($xmlDoc, $lineNumber);
        $lineNumber++;
        xmlwriter_end_attribute($xmlDoc);
        xmlwriter_start_attribute($xmlDoc, 'opcode');
        xmlwriter_text($xmlDoc, strtoupper($opcode));
        xmlwriter_end_attribute($xmlDoc);
        #echo $opcode."\n";
        return $arr_opcodes[strtoupper($opcode)];
    }
    
}

function parse_arguments($xmlDoc, $argString, $expectedArgs)
{
    if(!array_key_exists(1, $argString) && $expectedArgs != NULL)
    {
        #echo "No arguments specified\n";
        exit(23);
    }
    elseif(array_key_exists(1, $argString) && $expectedArgs == NULL)
    {
        if($argString[1][0] == '#')
        {
            return;
        }
        #echo "Unexpected argument\n";
        exit(23);
    }
    elseif(!array_key_exists(1, $argString) && $expectedArgs == NULL)
    {
        return;
    }
    #$argString[1] = rtrim($argString[1]);
    $argArray = explode(" ", $argString[1]);

    #Match patterns
    $patterns = [
        "variable" => '/(GF|LF|TF)@([A-Za-z]|[-_\$&%\*!\?])([A-Za-z0-9-\$&%\*!\?]*)/',
        "integer"  => '/int@\S*/',
        "boolean"  => '/bool@(true|false)/',
        "string"   => '/string@[\S^#]*/',
        "nil"      => '/nil@nil/',
        "type"     => '/(int|bool|string|nil)/',
        "label"    => '/([A-Za-z]|[-\$&%\*!\?])([A-Za-z0-9-\$&%\*!\?]*)/'
    ];
    $match = [];
    $expectedArgCount = count($expectedArgs);
    $argCount = 1;
    $value = "";
    foreach($argArray as $item)
    {
        
        if($argCount > $expectedArgCount)
        {
            #echo "Invalid argument count\n";
            exit(23);
        }
        xmlwriter_start_element($xmlDoc, 'arg'.$argCount);      #Start element argx
        xmlwriter_start_attribute($xmlDoc, 'type');                #Start attribute type
        if(preg_match($patterns["variable"], $item, $match))
        {
            #echo 'Found variable '.$match[0]."\n";
            if($expectedArgs[$argCount-1] == "var" || $expectedArgs[$argCount-1] == "symb")
            {
                xmlwriter_text($xmlDoc, 'var');
                xmlwriter_end_attribute($xmlDoc);                   #end attribute type
                xmlwriter_text($xmlDoc, $match[0]);
                #echo 'arg'.$argCount;
            }
            else
            {
                echo "Unexpected variable\n";
                exit(23);
            }
        }
        #Match integer literal
        elseif(preg_match($patterns["integer"], $item, $match))
        {
            if($expectedArgs[$argCount-1] == "symb")
            {
                $value = explode('@', $match[0])[1];
                xmlwriter_text($xmlDoc, 'int');
                xmlwriter_end_attribute($xmlDoc);                   #end attribute type
                xmlwriter_text($xmlDoc, $value);
                #echo 'Found integer '.$match[0]."\n";
            }
            else
            {
                #echo "Unexpected integer literal\n";
                exit(23);
            }
        }
        #Match boolean literal
        elseif(preg_match($patterns["boolean"], $item, $match))
        {
            if($expectedArgs[$argCount-1] == "symb")
            {
                $value = explode('@', $match[0])[1];
                xmlwriter_text($xmlDoc, 'bool');
                xmlwriter_end_attribute($xmlDoc);                   #end attribute type
                xmlwriter_text($xmlDoc, $value);
                #echo 'Found boolean '.$match[0]."\n";
            }
            else
            {
                echo "Unexpected boolean literal\n";
                exit(23);
            }
        }
        #Match string
        elseif(preg_match($patterns["string"], $item, $match))
        {
            if($expectedArgs[$argCount-1] == "symb")
            {
                $value = explode('@', $match[0], 2)[1];
                xmlwriter_text($xmlDoc, 'string');
                xmlwriter_end_attribute($xmlDoc);                   #end attribute type
                xmlwriter_text($xmlDoc, parse_string($value));
                #echo 'Found string '.$match[0]."\n";
            }
            else
            {
                #echo "Unexpected string literal\n";
                exit(23);
            }
        }
        #Match nil
        elseif(preg_match($patterns["nil"], $item, $match))
        {
            if($expectedArgs[$argCount-1] == "symb")
            {
                $value = explode('@', $match[0])[1];
                xmlwriter_text($xmlDoc, 'nil');
                xmlwriter_end_attribute($xmlDoc);                   #end attribute type
                xmlwriter_text($xmlDoc, $value);
                #echo 'Found nil '.$match[0]."\n";
            }
            else
            {
                #echo "Unexpected nil value\n";
                exit(23);
            }
        }
        #Match type
        elseif(preg_match($patterns["type"], $item, $match))
        {
            if($expectedArgs[$argCount-1] == "type")
            {
                xmlwriter_text($xmlDoc, 'type');
                xmlwriter_end_attribute($xmlDoc);                   #end attribute type
                xmlwriter_text($xmlDoc, $match[0]);
                #echo 'Found type '.$match[0]."\n";
            }
            else
            {
                #echo "Unexpected type\n";
                exit(23);
            }
        }
        #Match label
        elseif(preg_match($patterns["label"], $item, $match))
        {
            if($expectedArgs[$argCount-1] == "label")
            {
                xmlwriter_text($xmlDoc, 'label');
                xmlwriter_end_attribute($xmlDoc);                   #end attribute type
                xmlwriter_text($xmlDoc, $match[0]);
                #echo 'Found label '.$match[0]."\n";
            }
            else
            {
                #echo "Unexpected label\n";
                exit(23);
            }
        }
        
        $argCount++;
        xmlwriter_end_element($xmlDoc);     #End element argx
    }
    
}

function parse_header($xmlDoc)
{
    $header = fgets(STDIN);
    $header = rtrim(explode('#', $header)[0]);
    if(strtolower($header) != ".ippcode19")
    {
        #echo("Invalid or missing header\n");
        exit(21);
    }
}

$extras = parse_command_line_args($argv);



$STATP = [
    "--loc"       => 0,
    "--comments"  => 0,
    "--labels"    => 0,
    "--jumps"     => 0
];

$xw = xmlwriter_open_memory();
xmlwriter_set_indent($xw, 1);
$res = xmlwriter_set_indent_string($xw, ' ');

xmlwriter_start_document($xw, '1.0', 'UTF-8'); #Generate XML header

xmlwriter_start_element($xw, 'program');  #Generate element program with attribute language=IPPcode19
xmlwriter_start_attribute($xw, 'language');
xmlwriter_text($xw, 'IPPcode19');
xmlwriter_end_attribute($xw);



parse_header($xw);

$lineNumber = 1;
while(!feof(STDIN))
{ 
    $line = fgets(STDIN);
    parse_line($line, $lineNumber, $xw, $STATP);
    
}
xmlwriter_end_element($xw);

xmlwriter_end_document($xw);
echo(xmlwriter_output_memory($xw));

if($extras != NULL)
{
    print_stats($STATP, $extras["mask"], $extras["file"]);
}

?>

import xml.etree.ElementTree as ET
import re
import sys
import codecs
import argparse
import fileinput

class InvalidXMLHeader(Exception):
    """Invalid XML root element"""

class VariableNotFoundException(Exception):
    """Invalid variable"""

class LabelDoesNotExist(Exception):
    """Given label does not exist"""

class InvalidOperandsException(Exception):
    """Invalid operands for arithmetic or logic instruction"""

class ArithmeticException(Exception):
    """Division by zero"""

class InvalidValueException(Exception):
    """Operand has invalid value"""

class InvalidStringOperation(Exception):
    """Error while parsing string"""

class InvalidOpcodeException(Exception):
    """Invalid opcode"""

class InvalidLabelException(Exception):
    """Label redefinition"""

class FrameDoesNotExist(Exception):
    """Frame does not exist"""

class MissingValueException(Exception):
    """Value is not set"""

class Program(object):
    def __init__(self, source=None, infile=None):
        self.instruction_list = []
        self.instruction_pointer = 0
        self.instr_count = 0
        self.label_dict = {}
        self.callstack = []
        self.symtable = Symbol_table_stack()
        self.infile = infile
        self.reader = XML_reader(source)
        self.stack = []
   
    def add_all_labels(self):
        for index, instruction in enumerate(self.instruction_list):
            if (isinstance(instruction, instr_LABEL)):
                if (instruction.label[1] in self.label_dict):
                    raise InvalidLabelException()
                self.label_dict[instruction.label[1]] = index

    def runall(self):
        self.reader.parse_file()
        self.instruction_list = self.reader.get_list()
        self.add_all_labels()
 #       for instr in self.instruction_list:
 #           instr.run(self)
        while(self.instruction_pointer < len(self.instruction_list)):
            self.instruction_list[self.instruction_pointer].run(self)
            self.instr_count += 1

class Symbol_table_stack(object):
    def __init__(self):
        self.global_frame_table = {} #Empty global symtable
        self.temporary_frame_table = None   #No temporary frame
        self.local_frame_table = []     #Empty STACK of local frames

    def create_frame(self):
        self.temporary_frame_table = {}
    
    def push_frame(self):
        if (self.temporary_frame_table == None):
            raise FrameDoesNotExist()
        else:
            self.local_frame_table.append(self.temporary_frame_table)
            self.temporary_frame_table = None

    def pop_frame(self):
        try:
            self.temporary_frame_table = self.local_frame_table.pop()
        except:
            raise FrameDoesNotExist()

    def get_frame(self, variable:str):
        var_frame = variable.split('@')[0]
        if (var_frame == "GF"):
            return self.global_frame_table
        elif (var_frame == "TF"):
            return self.temporary_frame_table
        elif (var_frame == "LF"):
            if not self.local_frame_table:
                raise FrameDoesNotExist()
            else:
                return self.local_frame_table[-1]
        else:
            raise FrameDoesNotExist()

    def symbol_table_insert(self, variable:str):
        frame = self.get_frame(variable)
        item = Symbol_table_item(variable)

        if (frame == None):
            raise FrameDoesNotExist()
        if (item.name in frame):
            raise VariableNotFoundException()
        
        frame[item.name] = item

    def symbol_table_find(self, variable:str, type=None):
        frame = self.get_frame(variable)
        name = variable.split('@', 1)[1]

        if (frame == None):
            raise FrameDoesNotExist()

        if (name in frame):
            item = frame[name]
            if (type != None and item.type != type):
                raise InvalidOperandsException()
            else:
                return item
        else:
            return None

class Symbol_table_item(object):
    def __init__(self, variable:str):
        self.name = variable.split('@', 1)[1]
        self.type = None
        self.value = None

    def set_type(self,type):
        self.type = type
    
    def set_value(self, value):
        self.value = value
    
    def get_value(self):
        if (self.value == None):
            raise MissingValueException()
        return self.value
    
    def get_type(self):
        return self.type

class Checker(object):

    @staticmethod
    def cast(symbol, type):
        if (type == 'int'):
            return int(symbol)
        elif (type == 'string'):
            return str(symbol)
        elif (type == 'bool'):
            return bool(symbol)
        elif (type == 'nil'):
            return False

    @staticmethod
    def check_arith(arg2, arg3, type:str, program:Program):
        if (arg2[0] == 'var'):
            symbol1 = program.symtable.symbol_table_find(arg2[1])
            if (symbol1 == None):
                raise VariableNotFoundException()
            if (symbol1.get_type() == None):
                raise MissingValueException()
            elif (symbol1.get_type() != type):
                raise InvalidOperandsException()
            symbol1_val = symbol1.get_value()
        elif (arg2[0] == type):
            symbol1_val = arg2[1]
        else:
            raise MissingValueException()


        if (arg3[0] == 'var'):
            symbol2 = program.symtable.symbol_table_find(arg3[1])
            if (symbol2 == None):
                raise VariableNotFoundException()
            if (symbol2.get_type() == None):
                raise MissingValueException()
            elif (symbol2.get_type() != type):
                raise InvalidOperandsException()
            symbol2_val = symbol2.get_value()    
        elif (arg3[0] == type):
            symbol2_val = arg3[1]
        else:
            raise MissingValueException()
        
        return (symbol1_val, symbol2_val)
    @staticmethod
    def check_rel(arg2, arg3, program:Program):
        if (arg2[0] == 'var'):
            symbol1 = program.symtable.symbol_table_find(arg2[1])
            if (symbol1 == None):
                raise VariableNotFoundException()
            symbol1_type = symbol1.get_type()
            symbol1_value = symbol1.get_value()
        else:
            symbol1_type = arg2[0]
            symbol1_value = Checker.cast(arg2[1], symbol1_type)

        if (arg3[0] == 'var'):
            symbol2 = program.symtable.symbol_table_find(arg3[1])
            if (symbol2 == None):
                raise VariableNotFoundException()
            symbol2_type = symbol2.get_type()
            symbol2_value = symbol2.get_value()
        else:
            symbol2_type = arg3[0]
            symbol2_value = Checker.cast(arg3[1], symbol2_type)
        

        return (symbol1_type, symbol1_value, symbol2_type, symbol2_value)

    
    @staticmethod
    def check_logic(arg2, arg3, program:Program):
        if (arg2[0] == 'var'):
            symbol1 = program.symtable.symbol_table_find(arg2[1])
            if (symbol1 == None):
                raise VariableNotFoundException()
            if (symbol1.get_type() != 'bool'):
                raise InvalidOperandsException()
            symbol1_value = symbol1.get_value()
        elif (arg2[0] == 'bool'):
            symbol1_value = arg2[1]
        else:
            raise InvalidOperandsException()
        
        if (arg3[0] == 'var'):
            symbol2 = program.symtable.symbol_table_find(arg3[1])
            if (symbol2 == None):
                raise VariableNotFoundException()
            if (symbol2.get_type() != 'bool'):
                raise InvalidOperandsException()
            symbol2_value = symbol2.get_value()
        elif (arg3[0] == 'bool'):
            symbol2_value = arg3[1]
        else:
            raise InvalidOperandsException()

        return (bool(symbol1_value), bool(symbol2_value))
    
    @staticmethod
    def check_string_op(arg2, arg3, program:Program):
        if (arg2[0] == 'var'):
            src_var = program.symtable.symbol_table_find(arg2[1])
            if (src_var == None):
                raise VariableNotFoundException()
            if (src_var.get_type() == None):
                raise MissingValueException()
            if (src_var.get_type() != 'string'):
                raise InvalidOperandsException()
            string = str(src_var.get_value())
        elif (arg2[0] == 'string'):
            string = str(arg2[1])
        else:
            raise InvalidOperandsException()

        if (arg3[0] == 'var'):
            num_var = program.symtable.symbol_table_find(arg3[1])
            if (num_var == None):
                raise VariableNotFoundException()
            if (num_var.get_type() == None):
                raise MissingValueException()
            if (num_var.get_type() != 'int'):
                raise InvalidOperandsException()
            num = int(num_var.get_value())
        elif (arg3[0] == 'int'):
            num = int(arg3[1])
        else:
            raise InvalidOperandsException()
        
        if (num < 0 or num > len(string)-1):
            raise InvalidStringOperation()
        
        return (string, num)
    
    @staticmethod
    def parse_string(string):
        """Source https://stackoverflow.com/questions/4020539/process-escape-sequences-in-a-string-in-python"""

        ESCAPE_SEQUENCE_RE = re.compile(r'''
            ( \\U........      # 8-digit hex escapes
            | \\u....          # 4-digit hex escapes
            | \\x..            # 2-digit hex escapes
            | \\[0-7]{1,3}     # Octal escapes
            | \\N\{[^}]+\}     # Unicode characters by name
            | \\[\\'"abfnrtv]  # Single-character escapes
            )''', re.UNICODE | re.VERBOSE)

        match_list = re.findall("\\\\\d\d\d", string)
        match_list = list(set(match_list))
        for seq in match_list:
            repl = '0x'  + hex(int(seq[1:]))[2:].zfill(2)
            repl = '\\' + repl[1:]
            string = string.replace(seq, repl)
    
        def decode_match(match):
            return codecs.decode(match.group(0), 'unicode-escape')
        
        return ESCAPE_SEQUENCE_RE.sub(decode_match, string)


class Instruction(object):
    def __init__(self):
        pass

    def run(self, program:Program):
        raise NotImplementedError()

class instr_MOVE(Instruction):
    def __init__(self, arg1, arg2):
        self.dest = arg1
        self.source = arg2

    def run(self, program:Program):
        if (self.dest[0] != 'var'):
            raise InvalidOpcodeException()
        if (self.source[0] == 'var'):
            src_var = program.symtable.symbol_table_find(self.source[1])
            dst_var = program.symtable.symbol_table_find(self.dest[1])

            if (src_var == None or dst_var == None):
                raise VariableNotFoundException()
            
            dst_var.set_type(src_var.get_type())
            dst_var.set_value(src_var.get_value())

        else:
            dst_var = program.symtable.symbol_table_find(self.dest[1])
            if(dst_var == None):
                raise VariableNotFoundException()
            
            src_type = self.source[0]
            src_value = self.source[1]

            if src_type == 'string':
                dst_var.set_type('string')
                if (src_value == None):
                    dst_var.set_value("")
                else:
                    src_value = Checker.parse_string(src_value)
                    dst_var.set_value(str(src_value))
            elif src_type == 'int':
                dst_var.set_type('int')
                dst_var.set_value(int(src_value))
            elif src_type == 'bool':
                dst_var.set_type('bool')
                dst_var.set_value(True if src_value == 'true' else False)
            elif src_type == 'nil':
                dst_var.set_type('nil')
                dst_var.set_value(src_value)
            else:
                raise InvalidOpcodeException()

        program.instruction_pointer += 1

class instr_CREATEFRAME(Instruction):
    def __init__(self):
        pass

    def run(self, program:Program):
        program.symtable.create_frame()
        program.instruction_pointer += 1

class instr_PUSHFRAME(Instruction):
    def __init__(self):
        pass

    def run(self, program:Program):
        program.symtable.push_frame()
        program.instruction_pointer += 1

class instr_POPFRAME(Instruction):
    def __init__(self):
        pass

    def run(self, program:Program):
        program.symtable.pop_frame()
        program.instruction_pointer += 1

class instr_DEFVAR(Instruction):
    def __init__(self, arg1):
        self.var = arg1

    def run(self, program:Program):
        if (self.var == None):
            raise InvalidOpcodeException()
        if (self.var[0] != 'var'):
            raise InvalidOpcodeException()
        program.symtable.symbol_table_insert(self.var[1])
        program.instruction_pointer += 1
    
class instr_CALL(Instruction):
    def __init__(self, arg1):
        self.label = arg1

    def run(self, program:Program):
        if (self.label == None):
            raise InvalidOpcodeException()
        if (self.label[0] != 'label'):
            raise InvalidOpcodeException()
        try:
            label_line = program.label_dict[self.label[1]]
            program.callstack.append(program.instruction_pointer+1)
            program.instruction_pointer = label_line
        except:
            raise LabelDoesNotExist()

class instr_RETURN(Instruction):
    def __init__(self):
        pass

    def run(self, program:Program):
        try:
            program.instruction_pointer = program.callstack.pop()
        except:
            raise MissingValueException()

class instr_PUSHS(Instruction):
    def __init__(self, arg1):
        self.symb = arg1
    
    def run(self, program:Program):
        if (self.symb == None):
            raise InvalidOpcodeException()
        if (self.symb[0] == 'var'):
            variable = program.symtable.symbol_table_find(self.symb[1])
            if (variable == None):
                raise VariableNotFoundException()
            symb_type = variable.get_type()
            symb_value = variable.get_value()
        elif (self.symb[0] in {'int', 'bool', 'string'}):
            symb_type = self.symb[0]
            symb_value = self.symb[1]
        else:
            raise InvalidOperandsException()
        
        program.stack.append((symb_type, symb_value))
        program.instruction_pointer += 1
    
class instr_POPS(Instruction):
    def __init__(self, arg1):
        self.var = arg1
    
    def run(self, program:Program):
        if (self.var == None):
            raise InvalidOpcodeException()
        if (self.var[0] != 'var'):
            raise InvalidOperandsException()
        variable = program.symtable.symbol_table_find(self.var[1])
        if (variable == None):
            raise VariableNotFoundException()
        if (not program.stack):
            raise MissingValueException()
        (symb_type, symb_value) = program.stack.pop()
        variable.set_type(symb_type)
        variable.set_value(symb_value)
        program.instruction_pointer += 1

class instr_ADD(Instruction):
    def __init__(self, arg1, arg2, arg3):
        self.var = arg1
        self.symb1 = arg2
        self.symb2 = arg3

    def run(self, program:Program):
        if (self.var[0] != 'var'):
            raise InvalidOperandsException()
        variable = program.symtable.symbol_table_find(self.var[1])
        if (variable == None):
            raise VariableNotFoundException()

        (symbol1_val, symbol2_val) = Checker.check_arith(self.symb1, self.symb2, 'int', program)


        variable.set_type('int')
        variable.set_value(int(symbol1_val) + int(symbol2_val))
        program.instruction_pointer += 1

class instr_SUB(Instruction):
    def __init__(self, arg1, arg2, arg3):
        self.var = arg1
        self.symb1 = arg2
        self.symb2 = arg3

    def run(self, program:Program):
        if (self.var[0] != 'var'):
            raise InvalidOperandsException()
        variable = program.symtable.symbol_table_find(self.var[1])
        if (variable == None):
            raise VariableNotFoundException()

        (symbol1_val, symbol2_val) = Checker.check_arith(self.symb1, self.symb2, 'int', program)


        variable.set_type('int')
        variable.set_value(int(symbol1_val) - int(symbol2_val))
        program.instruction_pointer += 1

class instr_MUL(Instruction):
    def __init__(self, arg1, arg2, arg3):
        self.var = arg1
        self.symb1 = arg2
        self.symb2 = arg3

    def run(self, program:Program):
        if (self.var[0] != 'var'):
            raise InvalidOperandsException()
        variable = program.symtable.symbol_table_find(self.var[1])
        if (variable == None):
            raise VariableNotFoundException()

        (symbol1_val, symbol2_val) = Checker.check_arith(self.symb1, self.symb2, 'int', program)


        variable.set_type('int')
        variable.set_value(int(symbol1_val) * int(symbol2_val))
        program.instruction_pointer += 1

class instr_IDIV(Instruction):
    def __init__(self, arg1, arg2, arg3):
        self.var = arg1
        self.symb1 = arg2
        self.symb2 = arg3

    def run(self, program:Program):
        if (self.var[0] != 'var'):
            raise InvalidOperandsException()
        variable = program.symtable.symbol_table_find(self.var[1])
        if (variable == None):
            raise VariableNotFoundException()

        (symbol1_val, symbol2_val) = Checker.check_arith(self.symb1, self.symb2, 'int', program)

        variable.set_type('int')
        if(int(symbol2_val) == 0):
            raise ArithmeticException()
        variable.set_value(int(symbol1_val) // int(symbol2_val))
        program.instruction_pointer += 1

class instr_LT(Instruction):
    def __init__(self, arg1, arg2, arg3):
        self.var = arg1
        self.symb1 = arg2
        self.symb2 = arg3

    def run(self, program:Program):
        if (self.var[0] != 'var'):
            raise InvalidOperandsException()
        
        variable = program.symtable.symbol_table_find(self.var[1])
        
        if (variable == None):
            raise VariableNotFoundException()

        (symbo1_type, symbol1_value, symbol2_type, symbol2_value) = Checker.check_rel(self.symb1, self.symb2, program)
        if (symbo1_type != symbol2_type):
            raise InvalidOperandsException()
        variable.set_type('bool')
        variable.set_value(bool(symbol1_value < symbol2_value))
        program.instruction_pointer += 1

class instr_GT(Instruction):
    def __init__(self, arg1, arg2, arg3):
        self.var = arg1
        self.symb1 = arg2
        self.symb2 = arg3

    def run(self, program:Program):
        if (self.var[0] != 'var'):
            raise InvalidOperandsException()
        
        variable = program.symtable.symbol_table_find(self.var[1])
        
        if (variable == None):
            raise VariableNotFoundException()

        (symbo1_type, symbol1_value, symbol2_type, symbol2_value) = Checker.check_rel(self.symb1, self.symb2, program)
        if (symbo1_type != symbol2_type):
            raise InvalidOperandsException()
        variable.set_type('bool')
        variable.set_value(bool(symbol1_value > symbol2_value))
        program.instruction_pointer += 1

class instr_EQ(Instruction):
    def __init__(self, arg1, arg2, arg3):
        self.var = arg1
        self.symb1 = arg2
        self.symb2 = arg3

    def run(self, program:Program):
        if (self.var[0] != 'var'):
            raise InvalidOperandsException()
        
        variable = program.symtable.symbol_table_find(self.var[1])
        
        if (variable == None):
            raise VariableNotFoundException()

        (symbol1_type, symbol1_value, symbol2_type, symbol2_value) = Checker.check_rel(self.symb1, self.symb2, program)
        if ((symbol1_type != 'nil' and symbol2_type != 'nil') and symbol1_type != symbol2_type):
            raise InvalidOperandsException() 
        variable.set_type('bool')
        variable.set_value(bool(symbol1_value == symbol2_value))
        program.instruction_pointer += 1

class instr_AND(Instruction):
    def __init__(self, arg1, arg2, arg3):
        self.var = arg1
        self.symb1 = arg2
        self.symb2 = arg3

    def run(self, program:Program):
        if (self.var[0] != 'var'):
            raise InvalidOperandsException()
        
        variable = program.symtable.symbol_table_find(self.var[1])
        
        if (variable == None):
            raise VariableNotFoundException()

        (symb1_value, symb2_value) = Checker.check_logic(self.symb1, self.symb2, program)
        variable.set_type('bool')
        variable.set_value(bool(symb1_value and symb2_value))
        program.instruction_pointer += 1

class instr_OR(Instruction):
    def __init__(self, arg1, arg2, arg3):
        self.var = arg1
        self.symb1 = arg2
        self.symb2 = arg3

    def run(self, program:Program):
        if (self.var[0] != 'var'):
            raise InvalidOperandsException()
        
        variable = program.symtable.symbol_table_find(self.var[1])
        
        if (variable == None):
            raise VariableNotFoundException()

        (symb1_value, symb2_value) = Checker.check_logic(self.symb1, self.symb2, program)
        variable.set_type('bool')
        variable.set_value(bool(symb1_value or symb2_value))
        program.instruction_pointer += 1

class instr_NOT(Instruction):
    def __init__(self, arg1, arg2):
        self.var = arg1
        self.symb1 = arg2

    def run(self, program:Program):
        if (self.var[0] != 'var'):
            raise InvalidOperandsException()
        variable = program.symtable.symbol_table_find(self.var[1])
        
        if (variable == None):
            raise VariableNotFoundException()
        

        if (self.symb1[0] == 'var'):
            symbol1 = program.symtable.symbol_table_find(self.symb1[1])
            if (symbol1 == None):
                raise VariableNotFoundException()
            if (symbol1.get_type() == None):
                raise MissingValueException()
            if (symbol1.get_type() != 'bool'):
                raise InvalidOperandsException()
            symbol1_value = symbol1.get_value()
        elif (self.symb1[0] == 'bool'):
            symbol1_value = self.symb1[1]
        else:
            raise InvalidOperandsException()

        variable.set_type('bool')
        variable.set_value(bool(not symbol1_value))
        program.instruction_pointer += 1

class instr_INT2CHAR(Instruction):
    def __init__(self, arg1, arg2):
        self.var = arg1
        self.symb = arg2

    def run(self, program:Program):
        if (self.var[0] != 'var'):
            raise InvalidOperandsException()
        dst_var = program.symtable.symbol_table_find(self.var[1])
        
        if (dst_var == None):
            raise VariableNotFoundException()

        if (self.symb[0] == 'var'):
            src_var = program.symtable.symbol_table_find(self.symb[1])
            if (src_var == None):
                raise VariableNotFoundException()
            if (src_var.get_type() == None):
                raise MissingValueException()
            if (src_var.get_type() != 'int'):
                raise InvalidOperandsException()
            value = int(src_var.get_value())
        elif (self.symb[0] == 'int'):
            value = int(self.symb[1])
        else:
            raise InvalidOperandsException()
        
        try:
            char = chr(value)
        except:
            raise InvalidStringOperation()

        dst_var.set_type('string')
        dst_var.set_value(char)
        program.instruction_pointer += 1

class instr_STRI2INT(Instruction):
    def __init__(self, arg1, arg2, arg3):
        self.var = arg1
        self.symb1 = arg2
        self.symb2 = arg3

    def run(self, program:Program):
        if (self.var[0] != 'var'):
            raise InvalidOperandsException()
        dst_var = program.symtable.symbol_table_find(self.var[1])
        if (dst_var == None):
            raise VariableNotFoundException()

        (string, num) = Checker.check_string_op(self.symb1, self.symb2, program)
        
        dst_var.set_type('int')
        dst_var. set_value(int(ord(string[num])))

        program.instruction_pointer += 1

class instr_READ(Instruction):
    def __init__(self, arg1, arg2):
        self.var = arg1
        self.type = arg2

    def run(self, program:Program):
        if (self.var[0] != 'var'):
            raise InvalidOperandsException()
        dst_var = program.symtable.symbol_table_find(self.var[1])
        if (dst_var == None):
            raise VariableNotFoundException()

        if (self.type[0] != 'type'):
            raise InvalidOperandsException()
        #import pdb; pdb.set_trace()
        if (self.type[1] == 'int'):
            try:
                read = int(program.infile.readline().rstrip('\n'))
            except:
                read = 0
            finally:
                dst_var.set_type('int')
                dst_var.set_value(read)
        elif (self.type[1] == 'string'):
            try:
                read = program.infile.readline().rstrip('\n')
            except:
                read = ""
            finally:
                dst_var.set_type('string')
                dst_var.set_value(read)
        elif (self.type[1] == 'bool'):
            read = program.infile.readline().rstrip('\n')
            dst_var.set_type('bool')
            if (read.upper() == 'TRUE'):
                dst_var.set_value(bool(True))
            else:
                dst_var.set_value(bool(False))
        else:
            raise InvalidOpcodeException()
        
        #print(read)

        program.instruction_pointer += 1
        
class instr_WRITE(Instruction):
    def __init__(self,arg1):
        self.arg1 = arg1
    
    def run(self, program:Program):
        if (self.arg1[0] == 'var'):
            var = program.symtable.symbol_table_find(self.arg1[1])
            if (var == None):
                raise VariableNotFoundException()
            if (var.get_type == None):
                raise MissingValueException()
            if (var.get_type() == 'bool'):
                string = 'true' if var.get_value() == True else 'false'
            elif (var.get_type() == 'nil'):
                string = ""
            else:
                string = str(var.get_value())

        elif (self.arg1[0] == 'nil'):
            string = ""
        else:
            string = str(self.arg1[1])
            string = str(Checker.parse_string(string))

        
        print(string, end="")
        sys.stdout.flush()
        program.instruction_pointer += 1
  
class instr_CONCAT(Instruction):
    def __init__(self, arg1, arg2, arg3):
        self.var = arg1
        self.symb1 = arg2
        self.symb2 = arg3

    def run(self, program:Program):
        if (self.var[0] != 'var'):
            raise InvalidOperandsException()
        
        variable = program.symtable.symbol_table_find(self.var[1])
        
        if (variable == None):
            raise VariableNotFoundException()
        
        (symbol1_value, symbol2_value) = Checker.check_arith(self.symb1, self.symb2, 'string', program)

        variable.set_type('string')
        variable.set_value(str(symbol1_value + symbol2_value))
        program.instruction_pointer += 1

class instr_STRLEN(Instruction):
    def __init__(self, arg1, arg2):
        self.var = arg1
        self.symb = arg2
    
    def run(self, program:Program):

        if (self.var[0] != 'var'):
            raise InvalidOperandsException()
        dst_var = program.symtable.symbol_table_find(self.var[1])
        if (dst_var == None):
            raise VariableNotFoundException()
        
        if (self.symb[0] == 'var'):
            src_var = program.symtable.symbol_table_find(self.symb[1])
            if (src_var == None):
                raise VariableNotFoundException()
            if (src_var.get_type() == None):
                raise MissingValueException()
            if (src_var.get_type() != 'string'):
                raise InvalidOperandsException()
            string = str(src_var.get_value())
        elif (self.symb[0] == 'string'):
            if (self.symb[1] != None):
                string = Checker.parse_string(self.symb[1])
            else:
                string = ""
        else:
            raise InvalidOperandsException()
        
        dst_var.set_type('int')
        dst_var.set_value(int(len(string)))

        program.instruction_pointer += 1

class instr_GETCHAR(Instruction):
    def __init__(self, arg1, arg2,arg3):
        self.var = arg1
        self.symb1 = arg2
        self.symb2 = arg3
    
    def run(self, program:Program):
        if (self.var[0] != 'var'):
            raise InvalidOperandsException()
        dst_var = program.symtable.symbol_table_find(self.var[1])
        if (dst_var == None):
            raise VariableNotFoundException()

        (string, num) = Checker.check_string_op(self.symb1, self.symb2, program)

        dst_var.set_type('string')
        dst_var.set_value(str(string[num]))
        program.instruction_pointer += 1

class instr_SETCHAR(Instruction):
    def __init__(self, arg1, arg2,arg3):
        self.var = arg1
        self.symb1 = arg2
        self.symb2 = arg3
    
    def run(self, program:Program):
        if (self.var[0] != 'var'):
            raise InvalidOperandsException()
        dst_var = program.symtable.symbol_table_find(self.var[1])
        if (dst_var == None):
            raise VariableNotFoundException()
        if (dst_var.get_type() != 'string'):
            raise InvalidOperandsException()
        dst_str = dst_var.get_value()

        if (self.symb1[0] == 'var'):
            num_var = program.symtable.symbol_table_find(self.symb1[1])
            if (num_var == None):
                raise VariableNotFoundException()
            if (num_var.get_type() != 'int'):
                raise InvalidOperandsException()
            num = int(num_var.get_value())
        elif (self.symb1[0] == 'int'):
            num = int(self.symb1[1])
        else:
            raise InvalidOperandsException()

        if (self.symb2[0] == 'var'):
            src_var = program.symtable.symbol_table_find(self.symb2[1])
            if (src_var == None):
                raise VariableNotFoundException()
            if (src_var.get_type() != 'string'):
                raise InvalidOperandsException()
            string = str(src_var.get_value())
        elif (self.symb2[0] == 'string'):
            string = str(self.symb2[1])
        else:
            raise InvalidOperandsException()
        
        if (num < 0 or num > len(dst_str)-1):
            raise InvalidStringOperation()
        if (len(string) < 1):
            raise InvalidStringOperation()
        #print(string, file=sys.stderr)
        dst_str_arr = list(dst_str)
        dst_str_arr[num] = string[0]
        dst_str = ''.join(dst_str_arr)
        dst_var.set_value(dst_str)
        program.instruction_pointer += 1
       
class instr_TYPE(Instruction):
    def __init__(self, arg1, arg2):
        self.var = arg1
        self.symb1 = arg2
    
    def run(self, program:Program):
        if (self.var[0] != 'var'):
            raise InvalidOperandsException()
        dst_var = program.symtable.symbol_table_find(self.var[1])
        if (dst_var == None):
            raise VariableNotFoundException()
        
        if (self.symb1[0] == 'var'):
            src_var = program.symtable.symbol_table_find(self.symb1[1])
            if (src_var == None):
                raise VariableNotFoundException()
            
            src_type = src_var.get_type()
            if (src_type == None):
                src_type = ''
        else:
            src_type = self.symb1[0]
        
        dst_var.set_type('string')
        dst_var.set_value(src_type)
        program.instruction_pointer += 1

class instr_LABEL(Instruction):
    def __init__(self, arg1):
        self.label = arg1
        
    def run(self, program:Program):
        program.instruction_pointer += 1

class instr_JUMP(Instruction):
    def __init__(self, arg1):
        self.label = arg1

    def run(self, program:Program):
        try:
            label_line = program.label_dict[self.label[1]]
            program.instruction_pointer = label_line
        except:
            raise LabelDoesNotExist()

class instr_JUMPIFEQ(Instruction):
    def __init__(self, arg1, arg2, arg3):
        self.label = arg1
        self.symb1 = arg2
        self.symb2 = arg3

    def run(self, program:Program):
        (symbo1_type, symb1_val, symbol2_type, symb2_val) = Checker.check_rel(self.symb1, self.symb2, program)
        if (symbo1_type != symbol2_type):
            raise InvalidOperandsException()
        if (symb1_val == symb2_val):
            try:
                label_line = program.label_dict[self.label[1]]
                program.instruction_pointer = label_line
            except:
                raise LabelDoesNotExist()
        else:
            program.instruction_pointer += 1

class instr_JUMPIFNEQ(Instruction):
    def __init__(self, arg1, arg2, arg3):
        self.label = arg1
        self.symb1 = arg2
        self.symb2 = arg3

    def run(self, program:Program):
        (symbo1_type, symb1_val, symbol2_type, symb2_val) = Checker.check_rel(self.symb1, self.symb2, program)
        if (symbo1_type != symbol2_type):
            raise InvalidOperandsException()
        if (symb1_val != symb2_val):
            try:
                label_line = program.label_dict[self.label[1]]
                program.instruction_pointer = label_line
            except:
                raise LabelDoesNotExist()
        else:
            program.instruction_pointer += 1

class instr_EXIT(Instruction):
    def __init__(self, arg1):
        self.symb = arg1

    def run(self, program:Program):
        if (self.symb[0] == 'var'):
            src_var = program.symtable.symbol_table_find(self.symb[1])
            if (src_var == None):
                raise VariableNotFoundException()
            if (src_var.get_type() == None):
                raise MissingValueException()
            if (src_var.get_type() != 'int'):
                raise InvalidOperandsException()
            exit_code = src_var.get_value()
        elif (self.symb[0] == 'int'):
            exit_code = self.symb[1]
        else:
            raise InvalidOperandsException()

        if (int(exit_code) < 0 or int(exit_code) > 49):
            raise InvalidValueException()
        
        exit(int(exit_code))

class instr_DPRINT(Instruction):
    def __init__(self, arg1):
        self.symb = arg1

    def run(self, program:Program):
        if (self.symb[0] == 'var'):
            src_var = program.symtable.symbol_table_find(self.symb[1])
            if (src_var == None):
                raise VariableNotFoundException()
            string = src_var.get_value()
        else:
            string = self.symb[1]

        print(string, file=sys.stderr)
        program.instruction_pointer += 1

class instr_BREAK(Instruction):
    def __init__(self):
        pass
    
    def run(self, program:Program):
        code_pos = program.instruction_pointer
        if (len(program.symtable.local_frame_table) > 0):
            local_frame_state = program.symtable.local_frame_table[0]
        else:
            local_frame_state = "Empty"
        global_frame_state = program.symtable.global_frame_table
        temporary_frame_state = program.symtable.temporary_frame_table
        inst_exec = program.instr_count

        string = f'Current line: {code_pos}\
                   \nState of local frame: {local_frame_state}\
                   \nState of global frame: {global_frame_state}\
                   \nState of temporary frame: {temporary_frame_state}\
                   \nTotal number of instructions executed: {inst_exec}\n'
        print(string, file=sys.stderr)

        program.instruction_pointer += 1
        

class XML_reader(object):
    def __init__(self, file):
        try:
            self.tree = ET.parse(file)
            self.root = self.tree.getroot()
            self.list = []
        except:
            print("Error opening file")
            exit(11)

    def check_header(self):
        if (self.root.tag != 'program'):
            raise InvalidXMLHeader()
        if ('language' not in self.root.attrib or self.root.attrib['language'] != 'IPPcode19'):
            raise InvalidXMLHeader()

    def sort_instructions(self):
        """Source: https://effbot.org/zone/element-sort.htm """
        def getkey(elem):
            return int(elem.attrib['order'])
        container = self.root
        container[:] = sorted(container, key=getkey)

    def parse_file(self):
        self.sort_instructions()
        self.check_header()
        for instruction in self.root:
            opcode = instruction.get('opcode')
            try:
                arg1 = (instruction[0].get('type'), instruction[0].text)
                #print(arg1)
            except:
                arg1 = None
            
            try:
                arg2 = (instruction[1].get('type'), instruction[1].text)
                #print(arg2)
            except:
                arg2 = None
            
            try:
                arg3 = (instruction[2].get('type'), instruction[2].text)
                #print(arg3)
            except:
                arg3 = None

            instr = self.select_instruction(opcode, arg1, arg2, arg3)
            self.list.append(instr)
    
    def check_syntax(self, arg1=None, arg2=None, arg3=None):
        valid_set = {'int', 'bool', 'string', 'nil', 'label', 'type', 'var'}
        if (arg1 != None):
            if (arg1[0] not in valid_set):
                raise InvalidOpcodeException()
        if (arg2 != None):
            if (arg2[0] not in valid_set):
                raise InvalidOpcodeException()
        if (arg3 != None):
            if (arg3[0] not in valid_set):
                raise InvalidOpcodeException()

            
    def select_instruction(self, opcode:str, arg1=None,arg2=None,arg3=None):
        self.check_syntax(arg1, arg2, arg3)
        opcode = opcode.upper()
        if (opcode == 'MOVE'):
            return instr_MOVE(arg1, arg2)
        elif (opcode == "CREATEFRAME"):
            return instr_CREATEFRAME()
        elif (opcode == "PUSHFRAME"):
            return instr_PUSHFRAME()
        elif (opcode == "POPFRAME"):
            return instr_POPFRAME()
        elif (opcode == 'DEFVAR'):
            return instr_DEFVAR(arg1)
        elif (opcode == "CALL"):
            return instr_CALL(arg1)
        elif (opcode == "RETURN"):
            return instr_RETURN()
        elif (opcode == "PUSHS"):
            return instr_PUSHS(arg1)
        elif (opcode == "POPS"):
            return instr_POPS(arg1)
        elif (opcode == "ADD"):
            return instr_ADD(arg1, arg2, arg3)
        elif (opcode == "SUB"):
            return instr_SUB(arg1, arg2, arg3)
        elif (opcode == "MUL"):
            return instr_MUL(arg1, arg2, arg3)
        elif (opcode == "IDIV"):
            return instr_IDIV(arg1, arg2, arg3)
        elif (opcode == "LT"):
            return instr_LT(arg1, arg2, arg3)
        elif (opcode == "GT"):
            return instr_GT(arg1, arg2, arg3)
        elif (opcode == "EQ"):
            return instr_EQ(arg1, arg2, arg3)
        elif (opcode == "AND"):
            return instr_AND(arg1, arg2, arg3)
        elif (opcode == "OR"):
            return instr_OR(arg1, arg2, arg3)
        elif (opcode == "NOT"):
            return instr_NOT(arg1, arg2)
        elif (opcode == "INT2CHAR"):
            return instr_INT2CHAR(arg1, arg2)
        elif (opcode == "STRI2INT"):
            return instr_STRI2INT(arg1, arg2, arg3)
        elif (opcode == "READ"):
            return instr_READ(arg1, arg2)
        elif (opcode == 'WRITE'):
            return instr_WRITE(arg1)
        elif (opcode == "CONCAT"):
            return instr_CONCAT(arg1, arg2, arg3)
        elif (opcode == "STRLEN"):
            return instr_STRLEN(arg1, arg2)
        elif (opcode == "GETCHAR"):
            return instr_GETCHAR(arg1, arg2, arg3)
        elif (opcode == "SETCHAR"):
            return instr_SETCHAR(arg1, arg2, arg3)
        elif (opcode == "TYPE"):
            return instr_TYPE(arg1, arg2)
        elif (opcode == "LABEL"):
            return instr_LABEL(arg1)
        elif (opcode == "JUMP"):
            return instr_JUMP(arg1)
        elif (opcode == "JUMPIFEQ"):
            return instr_JUMPIFEQ(arg1, arg2, arg3)
        elif (opcode == "JUMPIFNEQ"):
            return instr_JUMPIFNEQ(arg1, arg2, arg3)
        elif (opcode == "EXIT"):
            return instr_EXIT(arg1)
        elif (opcode == "DPRINT"):
            return instr_DPRINT(arg1)
        elif (opcode == "BREAK"):
            return instr_BREAK()
        else:
            raise InvalidOpcodeException()

    def get_list(self):
        return self.list
                


if __name__ == "__main__":
    parser = argparse.ArgumentParser("This script interprets XML represenation of IPPcode19 program.\nAt least one of the following arguments has to be entered. The empty one will be replaced by STDIN")
    parser.add_argument("--source", action="store", help="Specifies input XML representation")
    parser.add_argument("--input", action="store", help="Specifies the source of user input")
    args = parser.parse_args()
    #print(args.source)

    if (args.source == None and args.input == None):
        exit()

    if (args.input == None):
        infile = sys.stdin
    else:
        try:
            infile = open(args.input, 'rt')
        except:
            exit()
    
    if (args.source == None):
        source = sys.stdin
    else:
        source = args.source

    try:
        prog = Program(source, infile)
        prog.runall()
    except InvalidXMLHeader:
        exit(32)
    except InvalidOpcodeException:
        exit(32)
    except InvalidLabelException:
        exit(52)
    except LabelDoesNotExist:
        exit(52)
    except InvalidOperandsException:
        exit(53)
    except VariableNotFoundException:
        exit(54)
    except FrameDoesNotExist:
        exit(55)
    except MissingValueException:
        exit(56)
    except InvalidValueException:
        exit(57)
    except ArithmeticException:
        exit(57)
    except InvalidStringOperation:
        exit(58)

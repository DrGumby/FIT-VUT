%Read wav file

[pathstr, name, ext] = fileparts(mfilename('fullpath'));
location = pwd;
if(location != pathstr)
  cd pathstr;
endif
  

[s, Fs] = wavread('xvojan00.wav');
s = s';
time = length(s)/Fs;
binary_count = length(s)/16;


fprintf(stdout, "The sampling rate is: %d\n", Fs);
fprintf(stdout, "Length in seconds is: %d\n", time);
fprintf(stdout, "The sample count is: %d\n", length(s));
fprintf(stdout, "The binary number count is: %d\n", binary_count);

binary_j = 1;
binary_vector = [];
for i = 1:8:32000
  if (s(i) < 0)
    binary_vector(binary_j) = 0;
  else
    binary_vector(binary_j) = 1;
  endif
  binary_j = binary_j+1;
end
textfile = load('xvojan00.txt')


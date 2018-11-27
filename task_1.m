%Read wav file
 

[s, Fs] = audioread('xvojan00.wav');
s = s';
time = length(s)/Fs;
binary_count = length(s)/16;


fprintf(stdout, "The sampling rate is: %d\n", Fs);
fprintf(stdout, "Length in seconds is: %d\n", time);
fprintf(stdout, "The sample count is: %d\n", length(s));
fprintf(stdout, "The binary number count is: %d\n", binary_count);

binary_j = 1;
binary_vector = [];
for i = 8:16:32000
  if (s(i) < 0)
    binary_vector(binary_j) = 0;
  else
    binary_vector(binary_j) = 1;
  endif
  binary_j = binary_j+1;
end
textfile = load('xvojan00.txt');
textfile = textfile';
if (isequal(textfile, binary_vector))
  fprintf(stdout, "The binary vectors are equal\n");
else
  fprintf(stdout, "The binary vectors are not equal\n");
endif


scale = 1:(Fs*20e-3);
bin_Fs = Fs/16;
bin_scale = 1:(bin_Fs*20e-3);
bin_sig20 = binary_vector(bin_scale);
sig20 = s(scale);
bin_t = linspace(0, length(bin_sig20)/bin_Fs, length(bin_sig20));
t = linspace(0, length(sig20)/Fs, length(sig20));

plot(t, sig20);xlabel('t'); ylabel('s[n], symbols'); title('Exercise 2');
hold on;
stem(bin_t, bin_sig20);
hold off;
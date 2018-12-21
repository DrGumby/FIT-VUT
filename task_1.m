%Read wav file
 

[s, Fs] = audioread('xvojan00.wav');
s = s';
time = length(s)/Fs;
binary_count = length(s)/16;

%Exercise 1
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

%Exercise 2
scale = 1:(Fs*20e-3);
bin_Fs = Fs/16;
bin_scale = 1:(bin_Fs*20e-3);
bin_sig20 = binary_vector(bin_scale);
sig20 = s(scale);
bin_t = linspace(0, length(bin_sig20)/bin_Fs, length(bin_sig20));
t = linspace(0, length(sig20)/Fs, length(sig20));

figure;plot(t, sig20);grid;xlabel('t [s]'); ylabel('s[n], symbols'); title('Exercise 2');
hold on;
stem(bin_t, bin_sig20);
hold off;
print -depsc ex2.eps;

%Exercise 3
B = [0.0192   -0.0185   -0.0185    0.0192];
A = [1.0000   -2.8870    2.7997   -0.9113];


figure;zplane (B,A);  xlabel("Re"); ylabel("Im");title("Exercise 3");
print -depsc ex3.eps;
p = roots(A); 
if (isempty(p) | abs(p) < 1) 
  disp('Filter is stable')
else
  disp('Filter is not stable')
end

%Exercise 4
H = freqz(B,A,256); f=(0:255) / 256 * Fs / 2; 
figure;plot (f,abs(H)); grid; xlabel('f [Hz]'); ylabel('|H(f)|');title("Exercise 4");
print -depsc ex4.eps;
%TODO Cutoff frequency

%Exercise 5
ss = filter(B,A,s);

subplot(212);plot(1:length(s), s);
subplot(211);plot(1:length(ss),ss);

R = xcorr(s, ss);
[maxval, index] = max(R);
shift_index = index - length(ss);
fprintf(stdout, "Need to shift ss %d samples behind\n", shift_index);


%Exercise 6
ss_shifted = shift(ss, shift_index);

binary_j_shifted = 1;
binary_vector_shifted = [];
for i = 8:16:32000
  if (ss_shifted(i) < 0)
    binary_vector_shifted(binary_j_shifted) = 0;
  else
    binary_vector_shifted(binary_j_shifted) = 1;
  endif
  binary_j_shifted = binary_j_shifted+1;
end

figure;
ss_sig20 = ss(scale);
ss_t = linspace(0, length(ss_sig20)/Fs, length(ss_sig20));
ss_shifted_sig20 = ss_shifted(scale);
ss_shifted_t = linspace(0, length(ss_shifted_sig20)/Fs, length(ss_shifted_sig20));
plot(t, sig20, ss_t, ss_sig20, ss_shifted_t, ss_shifted_sig20);grid;xlabel('t [s]'); ylabel('s[n], ss[n], ss_{shifted}[n], symbols'); title('Exercise 6');
bin_ss_shifted = binary_vector_shifted(bin_scale);
bin_ss_t = linspace(0, length(bin_ss_shifted)/bin_Fs, length(bin_ss_shifted));
hold on;
stem(bin_ss_t, bin_ss_shifted);
hold off;
print -depsc ex6.eps;



%Exercise 7

error_vec = xor(binary_vector_shifted, binary_vector);
error_bits = sum(error_vec == 1);
fprintf(stdout, "There are %d incorrect binary symbols\n", error_bits);
error_percentage = error_bits/length(binary_vector_shifted) * 100;
fprintf(stdout, "That makes %d %% bits incorrect\n", error_percentage);


%Exercise 8
resolution = length(s);
S = abs(fft(s, resolution));
SS = abs(fft(ss, resolution));

P2 = abs(S/length(S));
P1 = P2(1:length(S)/2+1);
P1(2:end-1) = 2*P1(2:end-1);

Pss2 = abs(SS/length(SS));
Pss1 = Pss2(1:length(SS)/2+1);
Pss1(2:end-1) = 2*Pss1(2:end-1);

f = Fs*(0:(length(S)/2))/length(S);

figure;plot(f, P1);ylabel("X_s[n]"); xlabel("f [Hz]"); title("Exercise 8 pt. 1");
print -depsc ex8_1.eps
figure;plot(f, Pss1);ylabel("X_{ss}[n]"); xlabel("f [Hz]"); title("Exercise 8 pt. 2");
print -depsc ex8_2.eps


%Exercise 9

gmin = min(min(s)); gmax = max(max(s));
points = 256;
g = linspace(gmin, gmax, points);
Fg = zeros(size(g));
for ii = 1:points
  thisg = g(ii);
  Fg(ii) = sum(s <= thisg) / length(s);
end
plot(g, Fg);


deltag = g(2) - g(1);
pg = hist(s, g) / length(s) / deltag;
figure;plot(g, pg);ylabel("Probability density function"); xlabel("x"); title("Exercise 9");
print -depsc ex9.eps;
area = trapz(g, pg);
fprintf(stdout, "Area is %d\n", area);

%Exercise 10

Rk = xcorr(s, 'biased');
k = idivide(length(Rk), 2) - 50 : idivide(length(Rk), 2) + 50;
figure; plot(-50:50, Rk(k)); ylabel("R[k]"); xlabel("k"); title("Exercise 10");
print -depsc ex10.eps;

%Exercise 11

R0 = Rk(idivide(length(Rk), 2));
R1 = Rk(idivide(length(Rk), 2) +1);
R16 = Rk(idivide(length(Rk), 2) +16);
fprintf(stdout, "R[0] is %d\n", R0);
fprintf(stdout, "R[1] is %d\n", R1);
fprintf(stdout, "R[16] is %d\n", R16);

%Exercise 12


x_axis = linspace(min(s), max(s), 100);
[h,p,r] = hist2opt(s(1:length(s)-1), s(2:length(s)), x_axis);
imagesc(-x_axis, x_axis, p);
colorbar;
print -depsc ex12.eps;
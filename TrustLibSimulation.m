clear;
close all;
Time = 201;


%%%%%% SENSOR NODE %%%%%%

%% Good Interaction only
RT = 1;		% Best
NS = 1;		% Best
NL = 0;		% Best
Bat = 1;	% Best

TrustLevel_Eval_Sensor = zeros(1,Time);

for i=2:Time
	AG = RT*(1/(((1+NL) + (2-NS))^RT))*(1/Bat);
	TrustLevel_Eval_Sensor(1,i) = TrustLevel_Eval_Sensor(1,i-1) + AG;
end


%% Bad Interaction only
RT = -1;	% Worst
NS = 1;		% Best
NL = 0;		% Best
Bat = 1;	% Best

TrustLevel_RT_Eval_Sensor = zeros(1,Time*2);
TrustLevel_RT_Eval_Sensor(1,1:Time) = TrustLevel_Eval_Sensor;

for i=Time+1:(Time*2)
	AG = RT*(1/(((1+NL) + (2-NS))^RT))*(1/Bat);
	TrustLevel_RT_Eval_Sensor(1,i) = TrustLevel_RT_Eval_Sensor(1,i-1) + AG;

	if TrustLevel_RT_Eval_Sensor(1,i) <= 0
		TrustLevel_RT_Eval_Sensor(1,i) = 0;
	end
end


%% Bad Interaction + Bad Latency
RT = -1;	% Worst
NS = 1;		% Best
NL = 1;		% Worst
Bat = 1;	% Best

TrustLevel_NT_Eval_Sensor = zeros(1,Time*2);
TrustLevel_NT_Eval_Sensor(1,1:Time) = TrustLevel_Eval_Sensor;

for i=Time+1:(Time*2)
	AG = RT*(1/(((1+NL) + (2-NS))^RT))*(1/Bat);
	TrustLevel_NT_Eval_Sensor(1,i) = TrustLevel_NT_Eval_Sensor(1,i-1) + AG;

	if TrustLevel_NT_Eval_Sensor(1,i) <= 0
		TrustLevel_NT_Eval_Sensor(1,i) = 0;
	end
end


%% Bad Interaction + Bad Latency + Bad Signal
RT = -1;	% Worst
NS = 0;		% Worst
NL = 1;		% Worst
Bat = 1;	% Best
TrustLevel_NS_Eval_Sensor = zeros(1,Time*2);
TrustLevel_NS_Eval_Sensor(1,1:Time) = TrustLevel_Eval_Sensor;

for i=Time+1:(Time*2)
	AG = RT*(1/(((1+NL) + (2-NS))^RT))*(1/Bat);
	TrustLevel_NS_Eval_Sensor(1,i) = TrustLevel_NS_Eval_Sensor(1,i-1) + AG;

	if TrustLevel_NS_Eval_Sensor(1,i) <= 0
		TrustLevel_NS_Eval_Sensor(1,i) = 0;
	end
end


%% Bad Interaction + Bad Latency + Bad Signal + Bad Battery
RT = -1;	% Worst
NS = 0;		% Worst
NL = 1;		% Worst
Bat = 0.1;	% Worst
TrustLevel_Bat_Eval_Sensor = zeros(1,Time*2);
TrustLevel_Bat_Eval_Sensor(1,1:Time) = TrustLevel_Eval_Sensor;

for i=Time+1:(Time*2)
	AG = RT*(1/(((1+NL) + (2-NS))^RT))*(1/Bat);
	TrustLevel_Bat_Eval_Sensor(1,i) = TrustLevel_Bat_Eval_Sensor(1,i-1) + AG;

	if TrustLevel_Bat_Eval_Sensor(1,i) <= 0
		TrustLevel_Bat_Eval_Sensor(1,i) = 0;
	end
end


figure('Name', 'Evolution confiance noeud capteur','NumberTitle','off')
plot(TrustLevel_Eval_Sensor); hold on;
plot(TrustLevel_RT_Eval_Sensor); hold on;
plot(TrustLevel_NT_Eval_Sensor); hold on;
plot(TrustLevel_NS_Eval_Sensor); hold on;
plot(TrustLevel_Bat_Eval_Sensor);
legend({'Good Interaction only','Bad Interaction only', 'Bad Interaction + Bad Latency', 'Bad Interaction + Bad Latency + Bad Signal', 'Bad Interaction + Bad Latency + Bad Signal + Bad Battery'}, 'FontSize', 16, 'Location', 'southeast');
xlabel('Nombre iteractions');
ylabel('Confiance (%)');
set(gca,'fontsize',14);
axis([0 Time*2 0 100]);










%%%%%% ACTUATOR NODE %%%%%%

%% Good Interaction only
RA = 1;		% Best
NS = 1;		% Best
NL = 0;		% Best

TrustLevel_Eval_Act = zeros(1,Time);

for i=2:Time
	AG = RA*(1/(((1+NL) + (2-NS))^RA));
	TrustLevel_Eval_Act(1,i) = TrustLevel_Eval_Act(1,i-1) + AG;
end


%% Bad Interaction only
RA = -1;	% Worst
NS = 1;		% Best
NL = 0;		% Best
TrustLevel_RA_Eval_Act = zeros(1,Time*2);
TrustLevel_RA_Eval_Act(1,1:Time) = TrustLevel_Eval_Act;

for i=Time+1:(Time*2)
	AG = RA*(1/(((1+NL) + (2-NS))^RA));%RA * (1 / (1 + (1-NS)))^RA - (0.4^RA);
	TrustLevel_RA_Eval_Act(1,i) = TrustLevel_RA_Eval_Act(1,i-1) + AG;

	if TrustLevel_RA_Eval_Act(1,i) <= 0
		TrustLevel_RA_Eval_Act(1,i) = 0;
	end
end


%% Bad Interaction + Bad Latency
RA = -1;	% Worst
NS = 1;		% Best
NL = 1;		% Worst

TrustLevel_NL_Eval_Act = zeros(1,Time*2);
TrustLevel_NL_Eval_Act(1,1:Time) = TrustLevel_Eval_Act;

for i=Time+1:(Time*2)
	AG = RA*(1/(((1+NL) + (2-NS))^RA));%RA * (1 / (1 + (1-NS)))^RA - (0.4^RA);
	TrustLevel_NL_Eval_Act(1,i) = TrustLevel_NL_Eval_Act(1,i-1) + AG;

	if TrustLevel_NL_Eval_Act(1,i) <= 0
		TrustLevel_NL_Eval_Act(1,i) = 0;
	end
end

%% Bad Interaction + Bad Latency + Bad Signal
RA = -1;	% Worst
NS = 0;		% Worst
NL = 1;		% Worst

TrustLevel_NS_Eval_Act = zeros(1,Time*2);
TrustLevel_NS_Eval_Act(1,1:Time) = TrustLevel_Eval_Act;

for i=Time+1:(Time*2)
	AG = RA*(1/(((1+NL) + (2-NS))^RA));%RA * (1 / (1 + (1-NS)))^RA - (0.4^RA);
	TrustLevel_NS_Eval_Act(1,i) = TrustLevel_NS_Eval_Act(1,i-1) + AG;

	if TrustLevel_NS_Eval_Act(1,i) <= 0
		TrustLevel_NS_Eval_Act(1,i) = 0;
	end
end


figure('Name', 'Evolution confiance actionneur','NumberTitle','off')
plot(TrustLevel_Eval_Act); hold on;
plot(TrustLevel_RA_Eval_Act); hold on;
plot(TrustLevel_NL_Eval_Act); hold on;
plot(TrustLevel_NS_Eval_Act);
legend({'Good Interaction only','Bad Interaction only', 'Bad Interaction + Bad Latency', 'Bad Interaction + Bad Latency + Bad Signal'}, 'FontSize', 16, 'Location', 'southeast');
xlabel('Nombre iteractions');
ylabel('Confiance (%)');
set(gca,'fontsize',14);
axis([0 Time*2 0 100]);



%%%%%% GATEWAY %%%%%%

%% Good Interaction only
I =  1;		% Best
NS = 1;		% Best
NF = 1;		% Best

TrustLevel_Eval_Gate = zeros(1,Time);

for i=2:Time
	AG = I*(1/(((2-NF) + (2-NS))^I));%I * (NF /(1+(1-NS)))^I - (0.4^I);
	TrustLevel_Eval_Gate(1,i) = TrustLevel_Eval_Gate(1,i-1) + AG;
end


%% Bad Interaction only
I = -1;		% Worst
NS = 1;		% Best
NF = 1;		% Best

TrustLevel_I_Eval_Gate = zeros(1,Time*2);
TrustLevel_I_Eval_Gate(1,1:Time) = TrustLevel_Eval_Gate;

for i=Time+1:(Time*2)
	AG = I*(1/(((2-NF) + (2-NS))^I));%I * (NF /(1+(1-NS)))^I - (0.4^I);
	TrustLevel_I_Eval_Gate(1,i) = TrustLevel_I_Eval_Gate(1,i-1) + AG;

	if TrustLevel_I_Eval_Gate(1,i) <= 0
		TrustLevel_I_Eval_Gate(1,i) = 0;
	end
end


%% Bad Interaction + Bad Signal
I = -1;		% Worst
NS = 0;		% Worst
NF = 1;		% Best

TrustLevel_NS_Eval_Gate = zeros(1,Time*2);
TrustLevel_NS_Eval_Gate(1,1:Time) = TrustLevel_Eval_Gate;

for i=Time+1:(Time*2)
	AG = I*(1/(((2-NF) + (2-NS))^I));%I * (NF /(1+(1-NS)))^I - (0.4^I);
	TrustLevel_NS_Eval_Gate(1,i) = TrustLevel_NS_Eval_Gate(1,i-1) + AG;

	if TrustLevel_NS_Eval_Gate(1,i) <= 0
		TrustLevel_NS_Eval_Gate(1,i) = 0;
	end
end

%% Bad Interaction + Bad Signal + Bad Fiability
I = -1;		% Worst
NS = 0;		% Worst
NF = 0;		% Worst

TrustLevel_NF_Eval_Gate = zeros(1,Time*2);
TrustLevel_NF_Eval_Gate(1,1:Time) = TrustLevel_Eval_Gate;

for i=Time+1:(Time*2)
	AG = I*(1/(((2-NF) + (2-NS))^I));%I * (NF /(1+(1-NS)))^I - (0.4^I);
	TrustLevel_NF_Eval_Gate(1,i) = TrustLevel_NF_Eval_Gate(1,i-1) + AG;

	if TrustLevel_NF_Eval_Gate(1,i) <= 0
		TrustLevel_NF_Eval_Gate(1,i) = 0;
	end
end


figure('Name', 'Evolution confiance passerelle','NumberTitle','off')
plot(TrustLevel_Eval_Gate); hold on;
plot(TrustLevel_I_Eval_Gate); hold on;
plot(TrustLevel_NS_Eval_Gate); hold on;
plot(TrustLevel_NF_Eval_Gate);
legend({'Good Interaction only','Bad Interaction only', 'Bad Interaction + Bad Signal', 'Bad Interaction + Bad Signal + Bad Fiability'}, 'FontSize', 16, 'Location', 'southeast');
xlabel('Nombre iteractions');
ylabel('Confiance (%)');
set(gca,'fontsize',14);
axis([0 Time*2 0 100]);







Time = 5000;

%% RANDOM X% GOOD & Y% BAD INTERACTIONS %%
Good = ones(1, 80*Time/100);
Bad = zeros(1, 20*Time/100);



for i=1:20*Time/100
	Bad(1,i) = -1;%quality(randperm(length(quality)));
end

quality = [Good,Bad];
for i=1:Time
	quality = quality(randperm(length(quality)));
end


%% SENSOR NODE %%
NL = 0;
Bat = 1;	% Best
NS = 1;
RT = quality;

TrustLevel_Eval_S = zeros(1,Time);
for i=2:Time
	AG = RT(1,i)*(1/(((1+NL) + (2-NS))^RT(1,i)))*(1/Bat);
	TrustLevel_Eval_S(1,i) = TrustLevel_Eval_S(1,i-1) + AG;

 	if TrustLevel_Eval_S(1,i) <= 0
 		TrustLevel_Eval_S(1,i) = 0;
 	elseif TrustLevel_Eval_S(1,i) >=100
 		TrustLevel_Eval_S(1,i) = 100;
 	end

end

% ACTUATOR NODE %%
RA = quality;
NS = 1;		% Best
NL = 0;		% Best


TrustLevel_Eval_A = zeros(1,Time);
for i=2:Time
	AG = RA(1,i)*(1/(((1+NL) + (2-NS))^RA(1,i)));
	TrustLevel_Eval_A(1,i) = TrustLevel_Eval_A(1,i-1) + AG;

 	if TrustLevel_Eval_A(1,i) <= 0
 		TrustLevel_Eval_A(1,i) = 0;
 	elseif TrustLevel_Eval_A(1,i) >=100
 		TrustLevel_Eval_A(1,i) = 100;
 	end

end


% GATEWAY %%
I =  quality;
NS = 1;		% Best
NF = 1;		% Best


TrustLevel_Eval_G = zeros(1,Time);
for i=2:Time
	AG = I(1,i)*(1/(((2-NF) + (2-NS))^I(1,i)));
	TrustLevel_Eval_G(1,i) = TrustLevel_Eval_G(1,i-1) + AG;

 	if TrustLevel_Eval_G(1,i) <= 0
 		TrustLevel_Eval_G(1,i) = 0;
 	elseif TrustLevel_Eval_G(1,i) >=100
 		TrustLevel_Eval_G(1,i) = 100;
 	end

end

figure('Name', 'Evolution confiance 80-20','NumberTitle','off')
plot(TrustLevel_Eval_S); hold on;
plot(TrustLevel_Eval_A); hold on;
plot(TrustLevel_Eval_G);
xlabel('Nombre iteractions');
ylabel('Confiance (%)');
axis([0 Time 0 100]);

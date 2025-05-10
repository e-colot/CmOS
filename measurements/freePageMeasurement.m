% freePageMeasurement.m

% Parameters
bitmapSize = 10000; % Size of the bitmap
capacities = (0:50:bitmapSize-1)/bitmapSize;
numTrials = 100000; % Number of trials for each capacity

% Initialize results
randomAccessTimes = zeros(size(capacities));
continueUntilFoundTimes = zeros(size(capacities));

% Loop over different capacities
h = waitbar(0, 'Processing capacities...'); % Initialize progress bar
for cIdx = 1:length(capacities)
    capacity = capacities(cIdx);
    numOnes = round(capacity * bitmapSize);
    
    % Generate bitmap with given capacity
    bitmap = [ones(1, numOnes), zeros(1, bitmapSize - numOnes)];
    bitmap = bitmap(randperm(bitmapSize)); % Shuffle the bitmap
    
    % Random Access Method
    randomAccessStart = tic;
    for trial = 1:numTrials
        while true
            idx = randi(bitmapSize); % Random index
            if bitmap(idx) == 0
                break;
            end
        end
    end
    randomAccessTimes(cIdx) = toc(randomAccessStart);
    
    % Continue Until Found Method
    continueUntilFoundStart = tic;
    for trial = 1:numTrials
        idx = randi(bitmapSize); % Random starting index
        while bitmap(idx) ~= 0
            idx = mod(idx, bitmapSize) + 1; % Increment index (wrap around)
        end
    end
    continueUntilFoundTimes(cIdx) = toc(continueUntilFoundStart);
    
    % Update progress bar
    waitbar(cIdx / length(capacities), h);
end
close(h); % Close progress bar

% Plot results
figure;
semilogy(capacities*100, randomAccessTimes, '-o', 'DisplayName', 'Random Access');
hold on;
semilogy(capacities*100, continueUntilFoundTimes, '-x', 'DisplayName', 'Continue Until Found');
xlabel('Disk used (%)');
ylabel('Time Taken (Seconds)');
title('Comparison of Strategies to Find a Zero in Bitmap');
legend('Location', 'NorthWest');
grid on;
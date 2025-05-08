clear; close all; clc;

sizes = 2.^(4:9);

avgFillTime = zeros(1, length(sizes));
avgEraseTime = zeros(1, length(sizes));
avgFillTimeCA = zeros(1, length(sizes));
avgEraseTimeCA = zeros(1, length(sizes));

% Loop through each file in the directory
for i = 1:length(sizes)
    
    % Construct the file path for fill
    filePath = ['./data/fill', num2str(sizes(i))];
    
    % Call the loadData function with the file path
    [info, data] = loadData(filePath, 4);

    fillTime = data(1:2:end);
    eraseTime = data(2:2:end);

    avgFillTime(i) = mean(fillTime);
    avgEraseTime(i) = mean(eraseTime);
    
    % Construct the file path for fillCA
    filePathCA = ['./data/fillCA', num2str(sizes(i))];
    
    % Call the loadData function with the file path for fillCA
    [infoCA, dataCA] = loadData(filePathCA, 4);

    fillTimeCA = dataCA(1:2:end);
    eraseTimeCA = dataCA(2:2:end);

    avgFillTimeCA(i) = mean(fillTimeCA);
    avgEraseTimeCA(i) = mean(eraseTimeCA);
end

if (info(1) ~= infoCA(1))
    error('Disk sizes do not match');
end
if (info(4) ~= infoCA(4))
    error('Average file sizes do not match');
end
diskSize = info(1);
avgFileSize = info(4);
% Plot for average fill time comparison
figure;
semilogx(sizes, avgFillTime, '-o', 'DisplayName', 'Linked-List Allocation');
hold on;
semilogx(sizes, avgFillTimeCA, '-o', 'DisplayName', 'Contiguous Allocation');
hold off;
xlim([min(sizes) max(sizes)]);
xticks(sizes); % Set x-axis ticks to only show the values in sizes
xlabel('Page Size (bytes)');
ylabel('Average Fill Time (s)');
title(sprintf('Average Fill Time vs Page Size\nDisk Size: %d kB, Avg File Size: %d B', ...
    floor(diskSize / 1024), floor(avgFileSize)));
legend('show');

% Plot for average erase time comparison
figure;
semilogx(sizes, avgEraseTime, '-o', 'DisplayName', 'Linked-List Allocation');
hold on;
semilogx(sizes, avgEraseTimeCA, '-o', 'DisplayName', 'Contiguous Allocation');
hold off;
xlim([min(sizes) max(sizes)]);
xticks(sizes); % Set x-axis ticks to only show the values in sizes
xlabel('Page Size (bytes)');
ylabel('Average Erase Time (s)');
title(sprintf('Average Erase Time vs Page Size\nDisk Size: %d kB, Avg File Size: %d B', ...
    floor(diskSize / 1024), floor(avgFileSize)));
legend('show');




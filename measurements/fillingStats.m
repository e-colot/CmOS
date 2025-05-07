clear; close all; clc;

sizes = 2.^(4:9);

avgFillTime = zeros(1, length(sizes));
avgEraseTime = zeros(1, length(sizes));
% Loop through each file in the directory
for i = 1:length(sizes)
    
    % Construct the file path
    filePath = ['./data/fill', num2str(sizes(i))];
    
    % Call the loadData function with the file path
    [info, data] = loadData(filePath, 4);

    fillTime = data(1:2:end);
    eraseTime = data(2:2:end);

    avgFillTime(i) = mean(fillTime);
    avgEraseTime(i) = mean(eraseTime);
end

diskSize = info(1);
avgFileSize = info(4);

figure;
plot(sizes, avgFillTime, '-o', 'DisplayName', 'Average Fill Time');
hold on;
plot(sizes, avgEraseTime, '-o', 'DisplayName', 'Average Erase Time');
hold off;
xlabel('Page Size (bytes)');
ylabel('Time (s)');
title(sprintf('Average Fill and Erase Time vs Page Size\nDisk Size: %d kB, Avg File Size: %d B', ...
    floor(diskSize / 1024), floor(avgFileSize)));
legend('show');





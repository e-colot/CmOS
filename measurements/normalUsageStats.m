clear; close all; clc;

sizes = 2.^(4:9);

avgDataRate = zeros(1, length(sizes));
avgDiskUsage = zeros(1, length(sizes));
avgDataRateCA = zeros(1, length(sizes));
avgDiskUsageCA = zeros(1, length(sizes));

% Loop through each file in the directory
for i = 1:length(sizes)
    
    % Construct the file path for data rate
    filePath = ['./data/normalOp', num2str(sizes(i))];
    
    % Call the loadData function with the file path
    [info, data] = loadData(filePath, 4);

    dataRate = data(1:2:end);
    diskUsage = data(2:2:end);

    avgDataRate(i) = mean(dataRate);
    avgDiskUsage(i) = mean(diskUsage);
    
    % Construct the file path for data rate CA
    filePathCA = ['./data/normalOpCA', num2str(sizes(i))];
    
    % Call the loadData function with the file path for data rate CA
    [infoCA, dataCA] = loadData(filePathCA, 4);

    dataRateCA = dataCA(1:2:end);
    diskUsageCA = dataCA(2:2:end);

    avgDataRateCA(i) = mean(dataRateCA);
    avgDiskUsageCA(i) = mean(diskUsageCA);
end

if (info(1) ~= infoCA(1))
    error('Disk sizes do not match');
end
if (info(4) ~= infoCA(4))
    error('Average file sizes do not match');
end
diskSize = info(1);
avgFileSize = info(4);

% Plot for average data rate comparison

figure;
yyaxis left;
semilogx(sizes, avgDataRate/1e3, '--', 'Color', [0 0.4470 0.7410], 'DisplayName', 'Data Rate (Linked allocation)', 'LineWidth', 1.5);
hold on;
semilogx(sizes, avgDataRateCA/1e3, '--', 'Color', [0.8500 0.3250 0.0980], 'DisplayName', 'Data Rate (Contiguous allocation)', 'LineWidth', 1.5);
ylabel('Average Data Rate (kB/s)');
xlabel('Page Size (Bytes)');
legend('show');
grid on;

yyaxis right;
semilogx(sizes, avgDiskUsage, '-', 'Color', [0 0.4470 0.7410], 'DisplayName', 'Efficiency % (Linked allocation)', 'LineWidth', 1.5);
hold on;
semilogx(sizes, avgDiskUsageCA, '-', 'Color', [0.8500 0.3250 0.0980], 'DisplayName', 'Efficiency % (Contiguous allocation)', 'LineWidth', 1.5);
ylabel('Storage efficiency (%)');
xlim([min(sizes) max(sizes)]);
xticks(sizes); % Set x-axis ticks to only show the values in sizes
title(sprintf('Comparison of Data Rate and Storage Efficiency\nDisk Size: %d kB, Avg File Size: %d B', ...
    floor(diskSize / 1024), floor(avgFileSize)));
ax = gca;
ax.YAxis(1).Color = 'k';
ax.YAxis(2).Color = 'k';
legend('show', 'Location', 'northwest');




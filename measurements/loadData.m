function [info, data] = loadData(filePath, infoLength)
    % loadData - Load data from a binary file and extract info and data vectors.
    

    try
        % Open the file in read mode
        fileID = fopen(filePath, 'rb');
        
        if fileID == -1
            error('Could not open the file. Please check the file path.');
        end
        
        % Read the file content as bytes
        fileBytes = fread(fileID, '*uint8');
        
        % Close the file
        fclose(fileID);

        % Extract the first 24 bytes as 'info' (infoLength unsigned integers of 8 bytes each)
        if length(fileBytes) < 8*infoLength
            error('File does not contain enough data for the info vector.');
        end
        info = typecast(fileBytes(1:8*infoLength), 'uint64');
        
        % The remaining bytes are the data, interpreted as doubles (8 bytes per value)
        if mod(length(fileBytes) - 8*infoLength, 8) ~= 0
            error('The remaining bytes are not a multiple of 8, cannot interpret as double.');
        end
        data = typecast(fileBytes(8*infoLength+1:end), 'double');
     
    catch ME
        % Handle errors and display the error message
        error(['Error: ', ME.message]);
    end
end

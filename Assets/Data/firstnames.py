import pandas as pd

# Load the provided CSV file
firstnames_file_path = 'FirstNamesTable_Old.csv'
firstnames_df = pd.read_csv(firstnames_file_path)

# Check if 'RowName' column exists and drop it if it does
if 'RowName' in firstnames_df.columns:
    firstnames_df.drop(columns=['RowName'], inplace=True)

# Add a new first column that is an all-lowercase unique copy of the NameValue with an index to ensure uniqueness
firstnames_df.insert(0, 'RowName', firstnames_df['NameValue'].str.lower() + '_' + firstnames_df.index.astype(str))

# Save the transformed dataframe to a new CSV file
output_path = 'FirstNamesTable.csv'
firstnames_df.to_csv(output_path, index=False)

# Display the first few rows of the transformed dataframe
print(firstnames_df.head())

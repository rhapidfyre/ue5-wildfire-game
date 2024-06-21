import pandas as pd

# Load the provided CSV file
file_path = 'combine.csv'
df = pd.read_csv(file_path)

# Combine rows with duplicate NameValue and similar PercentChance
def combine_similar_rows(df, tolerance=0.01):
    grouped = df.groupby('NameValue')
    combined_rows = []
    
    for name, group in grouped:
        group = group.sort_values(by='PercentChance')
        combined_row = group.iloc[0].copy()
        
        for i in range(1, len(group)):
            if abs(combined_row['PercentChance'] - group.iloc[i]['PercentChance']) <= tolerance:
                combined_row['EthnicGroups'] = f"{combined_row['EthnicGroups']},{group.iloc[i]['EthnicGroups']}"
                combined_row['EthnicGroups'] = ','.join(sorted(set(combined_row['EthnicGroups'].split(','))))
            else:
                combined_rows.append(combined_row)
                combined_row = group.iloc[i].copy()
        
        combined_rows.append(combined_row)
    
    return pd.DataFrame(combined_rows)

# Combine similar rows
combined_df = combine_similar_rows(df)

# Function to format EthnicGroups
def format_ethnic_groups(ethnic_groups):
    tags = ethnic_groups.split(',')
    formatted_tags = ','.join([f'(TagName="Game.Character.Race.{tag}")' for tag in tags])
    return f'"(GameplayTags=({formatted_tags}))"'

# Apply the formatting function to the EthnicGroups column
combined_df['EthnicGroups'] = combined_df['EthnicGroups'].apply(format_ethnic_groups)

# Save the transformed dataframe to a new CSV file
output_path = 'combined.csv'
combined_df.to_csv(output_path, index=False)

# Display the first few rows of the transformed dataframe
print(combined_df.head())

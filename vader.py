from vaderSentiment.vaderSentiment import SentimentIntensityAnalyzer

analyzer = SentimentIntensityAnalyzer()

# Test different cases for the word 'very'
sentence1 = "very strong"
sentence2 = "VERY strong"

print("Sentence 1:", sentence1)
print("Sentiment Score (lowercase):", analyzer.polarity_scores(sentence1))

print("Sentence 2:", sentence2)
print("Sentiment Score (uppercase):", analyzer.polarity_scores(sentence2))

import os
import chromadb
from chromadb.utils.embedding_functions import SentenceTransformerEmbeddingFunction
from sentence_transformers import SentenceTransformer
from langchain.text_splitter import RecursiveCharacterTextSplitter
import subprocess

# Settings
REPO_PATH = os.path.abspath(".")  # current directory
COLLECTION_NAME = "kernel-code"
MODEL = "llama3"

# Set up ChromaDB client
client = chromadb.PersistentClient(path="chroma_db")
embedding_fn = SentenceTransformerEmbeddingFunction(model_name="all-MiniLM-L6-v2")

collection = client.get_or_create_collection(name=COLLECTION_NAME, embedding_function=embedding_fn)

# Step 1: Index source files
def read_repo_files():
    texts = []
    ids = []
    for root, _, files in os.walk(REPO_PATH):
        for f in files:
            if f.endswith((".c", ".h", ".asm" ".log")):
                path = os.path.join(root, f)
                with open(path, "r", encoding="utf-8", errors="ignore") as file:
                    content = file.read()
                    texts.append(content)
                    ids.append(path.replace(REPO_PATH, ""))
    return ids, texts

def split_and_index(ids, docs):
    splitter = RecursiveCharacterTextSplitter(chunk_size=512, chunk_overlap=64)
    all_chunks = []
    chunk_ids = []
    for i, text in enumerate(docs):
        chunks = splitter.split_text(text)
        for j, chunk in enumerate(chunks):
            all_chunks.append(chunk)
            chunk_ids.append(f"{ids[i]}_chunk_{j}")
    collection.add(ids=chunk_ids, documents=all_chunks)
    print(f"Indexed {len(all_chunks)} chunks.")

# Step 2: Query
def search_context(query: str, k=4):
    results = collection.query(query_texts=[query], n_results=k)
    return results["documents"][0]

# Step 3: Call Ollama via subprocess
def query_ollama(prompt: str) -> str:
    result = subprocess.run(
        ["ollama", "run", MODEL],
        input=prompt.encode("utf-8"),
        stdout=subprocess.PIPE,
        stderr=subprocess.DEVNULL,
    )
    return result.stdout.decode("utf-8")

# Step 4: Main interaction
def main():
    print("Indexing files...")
    ids, docs = read_repo_files()
    if len(collection.get()['ids']) == 0:  # index only if empty
        split_and_index(ids, docs)

    while True:
        question = input("\n💬 Ask about your kernel code (or 'exit'): ")
        if question.strip().lower() == "exit":
            break

        chunks = search_context(question)
        context = "\n---\n".join(chunks)
        prompt = f"You are a kernel developer. Use the following code snippets to answer:\n{context}\n\nQuestion: {question}"
        print("\n🤖 Ollama Answer:")
        print(query_ollama(prompt))

if __name__ == "__main__":
    main()

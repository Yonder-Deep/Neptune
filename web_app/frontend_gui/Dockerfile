FROM node:20-slim

WORKDIR /app

# Install dependencies first (for better caching)
COPY package*.json ./
RUN npm ci

# Copy source code
COPY . .

# Default command: run dev server
CMD ["npm", "run", "dev", "--", "--host"]
